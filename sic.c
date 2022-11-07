/*
  +----------------------------------------------------------------------+
  | Copyright (c) 2018-2022 Bumble Inc.                                  |
  +----------------------------------------------------------------------+    
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

  +----------------------------------------------------------------------+
  | Author: Antony Dovgal <tony@team.bumble.com>                         |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "SAPI.h"
#include "ext/standard/info.h"
#include "php_sic.h"

#include <pthread.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <stdbool.h>
#if PHP_VERSION_ID >= 80000
# include "sic_arginfo.h"
#else
# include "sic_legacy_arginfo.h"
#endif

ZEND_DECLARE_MODULE_GLOBALS(sic)

/* {{{ PHP_INI
 */
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("sic.enabled", "0", PHP_INI_SYSTEM, OnUpdateBool, enabled, zend_sic_globals, sic_globals)
    STD_PHP_INI_ENTRY("sic.shard_num", "10", PHP_INI_SYSTEM, OnUpdateLongGEZero, shard_num, zend_sic_globals, sic_globals)
    STD_PHP_INI_ENTRY("sic.shard_size", "1M", PHP_INI_SYSTEM, OnUpdateLongGEZero, shard_size, zend_sic_globals, sic_globals)
PHP_INI_END()
/* }}} */

/* {{{ structs */
typedef struct {
	size_t oom_err_cnt;
	size_t frag_err_cnt;
} sic_errors_t;

typedef struct {
	size_t size;
	void *shm;
} sic_shard_t;

typedef struct _sic_entry_t sic_entry_t;

typedef struct {
    size_t size;
	zend_ulong key_hash;
	size_t key_len;
	zend_long value;
	time_t ttl;
	sic_entry_t *prev;
	sic_entry_t *next;
} sic_entry_header_t;

typedef struct _sic_entry_t {
	sic_entry_header_t h;
	char key[1];
} sic_entry_t;

typedef struct {
	sic_entry_t *head;
	size_t cnt;
	size_t data_size;
} sic_list_t;

typedef struct {
	pthread_rwlock_t lock;
	size_t size;
	size_t data_tail_offset;
	sic_errors_t errors;
	sic_list_t used;
	sic_list_t free;
} sic_shard_header_t;

typedef struct {
	sic_shard_t *shards;
	zend_long shard_num;
	size_t restart_cnt;
	time_t start_time;
	time_t last_restart_time;
} sic_t;

typedef union {
	void *p;
	int i;
	long l;
	double d;
	void (*f)(void);
} sic_word_t;

/* }}} */

static sic_t si_cache;

#define ALIGNSIZE(x, size) ((size) * (1 + (((x)-1)/(size))))
#define ALIGNWORD(x) ALIGNSIZE(x, sizeof(sic_word_t))

static int lock_attrs_initialized = 0;
static pthread_rwlockattr_t sic_lock_attr;

#define ENTRY_TOTAL_SIZE(key_len) \
	ALIGNWORD(sizeof(sic_entry_header_t) + (key_len) + 1)

#define ENTRY_SIZE(e) \
	(e)->h.size

#define ENTRY_KEY_SIZE(e) \
	(ENTRY_SIZE(e) - sizeof(sic_entry_header_t) - 1)

#define ENTRY_NEXT(e) \
	(e)->h.next

#define ENTRY_PREV(e) \
	(e)->h.prev

#define ENTRY_KEY(e) \
	(char *)(e)->key

#define ENTRY_KEY_LEN(e) \
	(e)->h.key_len

#define ENTRY_KEY_HASH(e) \
	(e)->h.key_hash

#define ENTRY_VAL(e) \
	(e)->h.value

#define ENTRY_TTL(e) \
	(e)->h.ttl

#define LIST_HEAD(list) \
	(list).head

#define LIST_CNT(list) \
	(list).cnt

#define LIST_DATA_SIZE(list) \
	(list).data_size

#define SHARD_UNUSED_SIZE(shard_header) \
	((shard_header)->size - (shard_header)->data_tail_offset)

#define LIST_REMOVE(list, e) \
	if (ENTRY_PREV(e)) { \
		assert(ENTRY_NEXT(ENTRY_PREV(e)) == (e)); \
		ENTRY_NEXT(ENTRY_PREV(e)) = ENTRY_NEXT(e); \
	} else { \
		LIST_HEAD(list) = ENTRY_NEXT(e); \
	} \
	if (ENTRY_NEXT(e)) { \
		ENTRY_PREV(ENTRY_NEXT(e)) = ENTRY_PREV(e); \
	} \
	LIST_CNT(list)--; \
	LIST_DATA_SIZE(list) -= ENTRY_SIZE(e); \
	ENTRY_NEXT(e) = NULL; \
	ENTRY_PREV(e) = NULL;

#define LIST_ADD(list, e) \
	assert(ENTRY_PREV(e) == NULL); \
	assert(ENTRY_NEXT(e) == NULL); \
	if (LIST_HEAD(list)) { \
		ENTRY_PREV(LIST_HEAD(list)) = (e); \
	} \
	ENTRY_PREV(e) = NULL; \
	ENTRY_NEXT(e) = LIST_HEAD(list); \
	LIST_HEAD(list) = (e); \
	LIST_CNT(list)++; \
	LIST_DATA_SIZE(list) += ENTRY_SIZE(e);

#define ENTRY_SET(e, key, key_len, key_hash, ttl, value) \
	assert(ENTRY_KEY_SIZE(e) >= (key_len)); \
	ENTRY_VAL(e) = (value); \
	ENTRY_KEY_LEN(e) = (key_len); \
	if (ttl > 0) { \
		ENTRY_TTL(e) = (zend_long)time(NULL) + ttl; \
	} else { \
		ENTRY_TTL(e) = 0; \
	} \
	ENTRY_KEY_HASH(e) = (key_hash); \
	memcpy(ENTRY_KEY(e), (key), (key_len) + 1 /* \0 */);

#define SHARD_DATA(shard, offset) \
	(((char *)(shard)->shm) + ALIGNWORD(sizeof(sic_shard_header_t) + offset))

#define SIC_DECLARE_VARS() \
    sigset_t oldmask, blockmask

#define SIC_HANDLE_BLOCK_INTERRUPTIONS() \
    sigfillset(&blockmask); \
    sigprocmask(SIG_BLOCK, &blockmask, &oldmask)

#define SIC_HANDLE_UNBLOCK_INTERRUPTIONS() \
	sigprocmask(SIG_SETMASK, &oldmask, NULL)

#define SIC_WLOCK_OR_RETURN(header, shard_id) \
	SIC_HANDLE_BLOCK_INTERRUPTIONS(); \
	if (!sic_shard_lock_write(header, shard_id)) { \
		SIC_HANDLE_UNBLOCK_INTERRUPTIONS(); \
		return -1; \
	}

#define SIC_RLOCK_OR_RETURN(header, shard_id) \
	SIC_HANDLE_BLOCK_INTERRUPTIONS(); \
	if (!sic_shard_lock_read(header, shard_id)) { \
		SIC_HANDLE_UNBLOCK_INTERRUPTIONS(); \
		return -1; \
	}


#define SIC_UNLOCK(header, shard_id) \
	sic_shard_lock_unlock(header, shard_id); \
	SIC_HANDLE_UNBLOCK_INTERRUPTIONS()

int sic_find_shard(zend_long shard_num, const char *key, const size_t key_len, zend_ulong *key_hash) /* {{{ */
{
	*key_hash = zend_inline_hash_func(key, key_len);
	return (*key_hash) % (zend_ulong)shard_num;
}
/* }}} */

static int _sic_entry_cmp(void *a, void *b) /* {{{ */
{
	return a < b;
}
/* }}} */

static void _sic_entry_swap(void *a, void *b) /* {{{ */
{
	sic_entry_t **ea = (sic_entry_t **)a;
	sic_entry_t **eb = (sic_entry_t **)b;
	sic_entry_t *etmp;

	etmp = *ea;
	*ea = *eb;
	*eb = etmp;
}
/* }}} */

static int _sic_collect_garbage_no_lock(sic_shard_t *shard) { /* {{{ */
	sic_entry_t *e;
	sic_shard_header_t *header;

	header = (sic_shard_header_t *)shard->shm;

	if (!LIST_HEAD(header->used)) {
		/* just cleanup the data and start from the beginning */
		memset(SHARD_DATA(shard, 0), 0, header->size);
		memset(&header->used, 0, sizeof(header->used));
		memset(&header->free, 0, sizeof(header->free));
		header->data_tail_offset = 0;
		return 0;
	}

	/* ok, now we're gonna create an array of all entries, sort it and then move
	 * all the entries to the beginning of the data segment, freeing the tail */

	/* don't use emalloc(), it can bailout on OOM */
	sic_entry_t **entries = malloc(sizeof(void *) * LIST_CNT(header->used));
	if (!entries) {
		php_error_docref(NULL, E_WARNING, "sic: out of memory when trying to collect garbage");
		return -1;
	}

	int entries_cnt = 0;
	e = LIST_HEAD(header->used);
	while (e != NULL) {
		entries[entries_cnt] = e;
		e = ENTRY_NEXT(e);
		entries_cnt++;
	}

	/* sort the entries by their address/offset */
	zend_sort(entries, entries_cnt, sizeof(sic_entry_t *), (compare_func_t) _sic_entry_cmp, (swap_func_t) _sic_entry_swap);

	/* move the entries to the beginning of the segment */
	e = NULL;
	header->data_tail_offset = 0;
	sic_entry_t *prev_e = NULL;
	int i;
	for (i = 0; i < entries_cnt; i++) {
		e = entries[i];
		if (SHARD_DATA(shard, header->data_tail_offset) < (char *)e) {
			memmove(SHARD_DATA(shard, header->data_tail_offset), e, ENTRY_SIZE(e));
		}

		e = (sic_entry_t *)SHARD_DATA(shard, header->data_tail_offset);

		/* fixup the entry size! this one is important - entry could've been written into a bigger slot */
		ENTRY_SIZE(e) = ENTRY_TOTAL_SIZE(ENTRY_KEY_LEN(e));

		/* rebuild the list on the go. just because we can */
		ENTRY_PREV(e) = prev_e;
		ENTRY_NEXT(e) = NULL;
		if (prev_e) {
			ENTRY_NEXT(prev_e) = e;
		} else {
			LIST_HEAD(header->used) = e;
		}
		header->data_tail_offset += ENTRY_SIZE(e);
		prev_e = e;
	}

	memset(&header->free, 0, sizeof(header->free));
	free(entries);

	if (e) {
		/* zero-fill unused space */
		memset(SHARD_DATA(shard, header->data_tail_offset), 0, SHARD_UNUSED_SIZE(header));
	}

	return 0;
}
/* }}} */

static bool sic_shard_lock_write(sic_shard_header_t *header, int shard_id) /* {{{ */
{
	int error = pthread_rwlock_wrlock(&header->lock);
	if (error != 0) {
		php_error_docref(NULL, E_WARNING, "pthread_rwlock_wrlock() failed: %d(%s) on shard %d (lock addr: %p)", error, strerror(errno), shard_id, &header->lock);
		return false;
	}

	return true;
} /* }}} */

static bool sic_shard_lock_read(sic_shard_header_t *header, int shard_id) /* {{{ */
{
	int error = pthread_rwlock_rdlock(&header->lock);
	if (error != 0) {
		php_error_docref(NULL, E_WARNING, "pthread_rwlock_rdlock() failed: %d(%s) on shard %d (lock addr: %p)", error, strerror(errno), shard_id, &header->lock);
		return false;
	}

	return true;
} /* }}} */

static bool sic_shard_lock_unlock(sic_shard_header_t *header, int shard_id) /* {{{ */
{
	int error = pthread_rwlock_unlock(&header->lock);
	if (error != 0) {
		php_error_docref(NULL, E_WARNING, "pthread_rwlock_unlock() failed: %d(%s) on shard %d (lock addr: %p)", error, strerror(errno), shard_id, &header->lock);
		return false;
	}

	return true;
} /* }}} */

static int sic_collect_garbage(sic_shard_t *shard, int shard_id) /* {{{ */
{
	sic_shard_header_t *header = (sic_shard_header_t *)shard->shm;
	SIC_DECLARE_VARS();

	SIC_WLOCK_OR_RETURN(header, shard_id);
	int res = _sic_collect_garbage_no_lock(shard);
	SIC_UNLOCK(header, shard_id);

	return res;
}
/* }}} */

static int _sic_entry_create_no_lock(sic_shard_t *shard, const char *key, const size_t key_len, const zend_ulong key_hash, const zend_long ttl, const zend_long value) /* {{{ */
{
	sic_entry_t *e;
	sic_shard_header_t *header;
	int gc_executed = 0;

	header = (sic_shard_header_t *)shard->shm;

	/* first try to find an unused entry */
	e = LIST_HEAD(header->free);
	while (e != NULL) {
		if (ENTRY_KEY_SIZE(e) >= key_len) {
			LIST_REMOVE(header->free, e);
			ENTRY_SET(e, key, key_len, key_hash, ttl, value);
			LIST_ADD(header->used, e);
			return 0;
		}
		e = ENTRY_NEXT(e);
	}

	/* okay, what if we have an entry of matching size that's already obsolete? */
	e = LIST_HEAD(header->used);
	time_t now = time(NULL);
	while (e != NULL) {
		if (ENTRY_TTL(e) == 0) {
			/* no ttl set */
		} else if (ENTRY_TTL(e) < now) {
			/* ttl is set and older than now */
			if (ENTRY_KEY_SIZE(e) >= key_len) {
				/* the entry fits our needs, reuse it */
				ENTRY_SET(e, key, key_len, key_hash, ttl, value);
				return 0;
			}
		}
		e = ENTRY_NEXT(e);
	}

	unsigned int entry_size = ENTRY_TOTAL_SIZE(key_len);

retry_after_gc:

	/* no unused entries, check if we have space */
	if (SHARD_UNUSED_SIZE(header) < entry_size) {
		if (LIST_HEAD(header->free)) {
			if (!gc_executed) {
				/* defrag the data, then try again */
				int res = _sic_collect_garbage_no_lock(shard);
				gc_executed = 1;
				if (res < 0) {
					/* well, there's not much we can do about it */
				} else {
					goto retry_after_gc;
				}
			}
			header->errors.frag_err_cnt++;
		} else {
			header->errors.oom_err_cnt++;
		}
		return -1;
	}

	/* we still have some unmarked space */
	e = (sic_entry_t *)SHARD_DATA(shard, header->data_tail_offset);
	memset(e, 0, entry_size);
	ENTRY_SIZE(e) = entry_size;
	ENTRY_SET(e, key, key_len, key_hash, ttl, value);
	LIST_ADD(header->used, e);

	size_t entry_end_offset = (char *)e - SHARD_DATA(shard, 0) + ENTRY_SIZE(e);

	if (header->data_tail_offset < entry_end_offset) {
		header->data_tail_offset = entry_end_offset;
	}

	return 0;
}
/* }}} */

static sic_entry_t *_sic_entry_get_no_lock(sic_shard_header_t *header, const char *key, const size_t key_len, const zend_ulong key_hash, int writing) /* {{{ */
{
	sic_entry_t *e;
	e = LIST_HEAD(header->used);
	time_t now = time(NULL);
	while (e != NULL) {
		if (key_hash == ENTRY_KEY_HASH(e) && key_len == ENTRY_KEY_LEN(e) && memcmp(key, ENTRY_KEY(e), key_len) == 0) {
			/* gotcha! */
			if (ENTRY_TTL(e) > 0 && ENTRY_TTL(e) < now) {
				if (writing) {
					LIST_REMOVE(header->used, e);
					LIST_ADD(header->free, e);
				} else {
					/* can't remove the entry here - we're under READ lock */
				}
				return NULL;
			}
			return e;
		}
		e = ENTRY_NEXT(e);
	}

	return NULL;
}
/* }}} */

int sic_entry_add_set(sic_t *cache, const char *key, const size_t key_len, const zend_long value, const zend_long ttl, int add) /* {{{ */
{
	sic_shard_header_t *header;
	sic_shard_t *shard;
	zend_ulong key_hash;
	SIC_DECLARE_VARS();

	int shard_num = sic_find_shard(si_cache.shard_num, key, key_len, &key_hash);

	shard = &(cache->shards[shard_num]);
	header = (sic_shard_header_t *)shard->shm;

	SIC_WLOCK_OR_RETURN(header, shard_num);

	/* look for existing entry */
	sic_entry_t *e = _sic_entry_get_no_lock(header, key, key_len, key_hash, 1);
	if (e) {
		if (add) {
			/* ADD failed, the entry already exists */
			SIC_UNLOCK(header, shard_num);
			return -1;
		}

		/* it's a SET, so just rewrite the value */
		ENTRY_VAL(e) = value;
		if (ttl > 0) {
			ENTRY_TTL(e) = (zend_long)time(NULL) + ttl;
		} else {
			ENTRY_TTL(e) = 0;
		}
		SIC_UNLOCK(header, shard_num);
		return 0;
	}

	/* create new entry */
	int res = _sic_entry_create_no_lock(shard, key, key_len, key_hash, ttl, value);
	SIC_UNLOCK(header, shard_num);
	return res;
}
/* }}} */

static int sic_entry_get(sic_t *cache, const char *key, const size_t key_len, zend_long *value) /* {{{ */
{
	sic_entry_t *e;
	sic_shard_header_t *header;
	sic_shard_t *shard;
	zend_ulong key_hash;
	SIC_DECLARE_VARS();

	int shard_num = sic_find_shard(si_cache.shard_num, key, key_len, &key_hash);

	shard = &(cache->shards[shard_num]);
	header = (sic_shard_header_t *)shard->shm;

	SIC_RLOCK_OR_RETURN(header, shard_num);

	e = _sic_entry_get_no_lock(header, key, key_len, key_hash, 0);
	if (e) {
		*value = ENTRY_VAL(e);
		SIC_UNLOCK(header, shard_num);
		return 0;
	}
	SIC_UNLOCK(header, shard_num);
	return -1;
}
/* }}} */

static int sic_entry_del(sic_t *cache, const char *key, const size_t key_len) /* {{{ */
{
	sic_entry_t *e;
	sic_shard_header_t *header;
	sic_shard_t *shard;
	zend_ulong key_hash;
	SIC_DECLARE_VARS();

	int shard_num = sic_find_shard(si_cache.shard_num, key, key_len, &key_hash);

	shard = &(cache->shards[shard_num]);
	header = (sic_shard_header_t *)shard->shm;

	SIC_WLOCK_OR_RETURN(header, shard_num);

	e = _sic_entry_get_no_lock(header, key, key_len, key_hash, 1);
	if (e) {
		LIST_REMOVE(header->used, e);
		LIST_ADD(header->free, e);
		SIC_UNLOCK(header, shard_num);
		return 0;
	}
	SIC_UNLOCK(header, shard_num);
	return -1;
}
/* }}} */

static int sic_entry_inc_dec(sic_t *cache, const char *key, const size_t key_len, zend_long ttl, zend_long *value, const zend_long inc_dec) /* {{{ */
{
	sic_entry_t *e;
	sic_shard_header_t *header;
	sic_shard_t *shard;
	zend_ulong key_hash;
	SIC_DECLARE_VARS();

	int shard_num = sic_find_shard(si_cache.shard_num, key, key_len, &key_hash);

	shard = &(cache->shards[shard_num]);
	header = (sic_shard_header_t *)shard->shm;

	SIC_WLOCK_OR_RETURN(header, shard_num);

	/* look for existing value */
	e = _sic_entry_get_no_lock(header, key, key_len, key_hash, 1);
	if (e) {
		ENTRY_VAL(e) += inc_dec;
		*value = ENTRY_VAL(e);
		SIC_UNLOCK(header, shard_num);
		return 0;
	}

	/* no such entry, create one */
	int res = _sic_entry_create_no_lock(shard, key, key_len, key_hash, ttl, inc_dec);
	/* set initial value */
	*value = inc_dec;
	SIC_UNLOCK(header, shard_num);
	return res;
}
/* }}} */

static int sic_entry_cas(sic_t *cache, const char *key, const size_t key_len, const zend_long old_val, const zend_long new_val) /* {{{ */
{
	sic_entry_t *e;
	sic_shard_header_t *header;
	sic_shard_t *shard;
	zend_ulong key_hash;
	SIC_DECLARE_VARS();

	int shard_num = sic_find_shard(si_cache.shard_num, key, key_len, &key_hash);

	shard = &(cache->shards[shard_num]);
	header = (sic_shard_header_t *)shard->shm;

	SIC_WLOCK_OR_RETURN(header, shard_num);

	/* look for existing value */
	e = _sic_entry_get_no_lock(header, key, key_len, key_hash, 1);
	if (e) {
		if (ENTRY_VAL(e) == old_val) {
			ENTRY_VAL(e) = new_val;
			SIC_UNLOCK(header, shard_num);
			return 0;
		}
	}

	SIC_UNLOCK(header, shard_num);
	return -1;
}
/* }}} */

static int sic_lock_init(pthread_rwlock_t *lock) /* {{{ */
{
	if (!lock_attrs_initialized) {
		if (pthread_rwlockattr_init(&sic_lock_attr) != 0) {
			php_error_docref(NULL, E_WARNING, "pthread_rwlockattr_init() failed: %s", strerror(errno));
			return -1;
		}

		if (pthread_rwlockattr_setpshared(&sic_lock_attr, PTHREAD_PROCESS_SHARED) != 0) {
			php_error_docref(NULL, E_WARNING, "pthread_rwlockattr_setpshared() failed: %s", strerror(errno));
			return -1;
		}
		lock_attrs_initialized = 1;
	}

	if (pthread_rwlock_init(lock, &sic_lock_attr) != 0) {
		php_error_docref(NULL, E_WARNING, "pthread_rwlockattr_setpshared() failed: %s", strerror(errno));
		return -1;
	}
	return 0;
}
/* }}} */

static int sic_shard_init(sic_shard_t *shard, size_t size) /* {{{ */
{
	shard->size = 0;
	shard->shm = (void *)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	if((long)shard->shm == -1) {
		php_error_docref(NULL, E_WARNING, "sic module: mmap() failed: %s", strerror(errno));
		return -1;
    }
	shard->size = size;

	sic_shard_header_t *header = (sic_shard_header_t *)shard->shm;
	if (sic_lock_init(&header->lock) < 0) {
		return -1;
	}
	/* memset(header, 0, sizeof(*header)); anonymous mmap is zero-initialized */
	header->size = size - ALIGNWORD(sizeof(sic_shard_header_t));

	return 0;
}
/* }}} */

static int sic_shard_free(sic_shard_t *shard) /* {{{ */
{
	if (munmap(shard->shm, shard->size) < 0) {
		php_error_docref(NULL, E_WARNING, "munmap() failed: %s", strerror(errno));
		return -1;
    }
	shard->size = 0;
	return 0;
}
/* }}} */


int sic_init(sic_t *sic, zend_long num, zend_long size) /* {{{ */
{
	zend_long i;

	sic->shard_num = 0;
	sic->shards = malloc(sizeof(sic_shard_t) * num);
	memset(sic->shards, 0, sizeof(sic_shard_t) * num);

	for (i = 0; i < num; i++) {
		if (sic_shard_init(&(sic->shards[i]), size) < 0) {
			return -1;
		}
		sic->shard_num++;
	}
	return 0;
}
/* }}} */

int sic_free(sic_t *sic) /* {{{ */
{
	zend_long i;
	for (i = 0; i < sic->shard_num; i++) {
		sic_shard_free(&(sic->shards[i]));
	}
	free(sic->shards);
	return 0;
}
/* }}} */


/* {{{ proto bool sic_set(string key, int value[, int ttl])
 Update existing entry or create a new one.
*/
PHP_FUNCTION(sic_set)
{
	char *key;
	size_t key_len;
	zend_long val;
	zend_long ttl = 0;

	ZEND_PARSE_PARAMETERS_START(2, 3)
		Z_PARAM_STRING(key, key_len)
		Z_PARAM_LONG(val)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG(ttl)
	ZEND_PARSE_PARAMETERS_END();

	if (!si_cache.shard_num) {
		php_error_docref(NULL, E_WARNING, "sic is not enabled");
		RETURN_FALSE;
	}

	if (sic_entry_add_set(&si_cache, key, key_len, val, ttl, 0) < 0) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool sic_add(string key, int value[, int ttl])
 Add a new entry if there's no entry with this key.
*/
PHP_FUNCTION(sic_add)
{
	char *key;
	size_t key_len;
	zend_long val;
	zend_long ttl = 0;

	ZEND_PARSE_PARAMETERS_START(2, 3)
		Z_PARAM_STRING(key, key_len)
		Z_PARAM_LONG(val)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG(ttl)
	ZEND_PARSE_PARAMETERS_END();

	if (!si_cache.shard_num) {
		php_error_docref(NULL, E_WARNING, "sic is not enabled");
		RETURN_FALSE;
	}

	if (sic_entry_add_set(&si_cache, key, key_len, val, ttl, 1) < 0) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool sic_del(string key)
 Delete existing entry.
 */
PHP_FUNCTION(sic_del)
{
	char *key;
	size_t key_len;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_STRING(key, key_len)
	ZEND_PARSE_PARAMETERS_END();

	if (!si_cache.shard_num) {
		php_error_docref(NULL, E_WARNING, "sic is not enabled");
		RETURN_FALSE;
	}

	if (sic_entry_del(&si_cache, key, key_len) < 0) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto int sic_get(string key)
 Fetch existing entry.
*/
PHP_FUNCTION(sic_get)
{
	char *key;
	size_t key_len;
	zend_long val;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_STRING(key, key_len)
	ZEND_PARSE_PARAMETERS_END();

	if (!si_cache.shard_num) {
		php_error_docref(NULL, E_WARNING, "sic is not enabled");
		RETURN_FALSE;
	}

	if (sic_entry_get(&si_cache, key, key_len, &val) < 0) {
		RETURN_FALSE;
	}
	RETURN_LONG(val);
}
/* }}} */

/* {{{ proto bool sic_exists(string key)
 Check if entry with such key exists.
*/
PHP_FUNCTION(sic_exists)
{
	char *key;
	size_t key_len;
	zend_long val;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_STRING(key, key_len)
	ZEND_PARSE_PARAMETERS_END();

	if (!si_cache.shard_num) {
		php_error_docref(NULL, E_WARNING, "sic is not enabled");
		RETURN_FALSE;
	}

	if (sic_entry_get(&si_cache, key, key_len, &val) < 0) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto int sic_inc(string key[, int inc_value[, int ttl]])
 Increment existing entry or create a new one with the value provided.
 Returns new value.
*/
PHP_FUNCTION(sic_inc)
{
	char *key;
	size_t key_len;
	zend_long inc_val = 1, val;
	zend_long ttl = 0;

	ZEND_PARSE_PARAMETERS_START(1, 3)
		Z_PARAM_STRING(key, key_len)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG(inc_val)
		Z_PARAM_LONG(ttl)
	ZEND_PARSE_PARAMETERS_END();

	if (!si_cache.shard_num) {
		php_error_docref(NULL, E_WARNING, "sic is not enabled");
		RETURN_FALSE;
	}

	if (sic_entry_inc_dec(&si_cache, key, key_len, ttl, &val, inc_val) < 0) {
		RETURN_FALSE;
	}
	RETURN_LONG(val);
}
/* }}} */

/* {{{ proto int sic_dec(string key[, int dec_value[, ttl]])
 Decrement existing entry or create a new one with the value provided.
 Returns new value.
 */
PHP_FUNCTION(sic_dec)
{
	char *key;
	size_t key_len;
	zend_long dec_val = 1, val;
	zend_long ttl = 0;

	ZEND_PARSE_PARAMETERS_START(1, 3)
		Z_PARAM_STRING(key, key_len)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG(dec_val)
		Z_PARAM_LONG(ttl)
	ZEND_PARSE_PARAMETERS_END();

	if (!si_cache.shard_num) {
		php_error_docref(NULL, E_WARNING, "sic is not enabled");
		RETURN_FALSE;
	}

	if (sic_entry_inc_dec(&si_cache, key, key_len, ttl, &val, - dec_val) < 0) {
		RETURN_FALSE;
	}
	RETURN_LONG(val);
}
/* }}} */

/* {{{ proto bool sic_cas(string key, int old_value, int new_value)
 Update existing entry if its value is the one we expect to see.
*/
PHP_FUNCTION(sic_cas)
{
	char *key;
	size_t key_len;
	zend_long old_val, new_val;

	ZEND_PARSE_PARAMETERS_START(3, 3)
		Z_PARAM_STRING(key, key_len)
		Z_PARAM_LONG(old_val)
		Z_PARAM_LONG(new_val)
	ZEND_PARSE_PARAMETERS_END();

	if (!si_cache.shard_num) {
		php_error_docref(NULL, E_WARNING, "sic is not enabled");
		RETURN_FALSE;
	}

	if (sic_entry_cas(&si_cache, key, key_len, old_val, new_val) < 0) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool sic_gc()
 Run GC routine.
*/
PHP_FUNCTION(sic_gc)
{
	ZEND_PARSE_PARAMETERS_START(0, 0)
	ZEND_PARSE_PARAMETERS_END();

	if (!si_cache.shard_num) {
		php_error_docref(NULL, E_WARNING, "sic is not enabled");
		RETURN_FALSE;
	}

	zend_long i;
	for (i = 0; i < si_cache.shard_num; i++) {
		sic_collect_garbage(&si_cache.shards[i], i);
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto array sic_info()
 Get info about the shards.
*/
PHP_FUNCTION(sic_info)
{
	ZEND_PARSE_PARAMETERS_START(0, 0)
	ZEND_PARSE_PARAMETERS_END();

	if (!si_cache.shard_num) {
		php_error_docref(NULL, E_WARNING, "sic is not enabled");
		RETURN_FALSE;
	}

	array_init(return_value);

	zend_long i;
	for (i = 0; i < si_cache.shard_num; i++) {
		zval shard;

		array_init(&shard);
		sic_shard_header_t *header = (sic_shard_header_t *)si_cache.shards[i].shm;

		add_assoc_long(&shard, "size", header->size);
		add_assoc_long(&shard, "unused_size", SHARD_UNUSED_SIZE(header));
		add_assoc_long(&shard, "used_cnt", header->used.cnt);
		add_assoc_long(&shard, "used_data_size", header->used.data_size);
		add_assoc_long(&shard, "free_cnt", header->free.cnt);
		add_assoc_long(&shard, "free_data_size", header->free.data_size);
		add_assoc_long(&shard, "frag_err_cnt", header->errors.frag_err_cnt);
		add_assoc_long(&shard, "oom_err_cnt", header->errors.oom_err_cnt);
		add_next_index_zval(return_value, &shard);
	}
}
/* }}} */

/* {{{ php_sic_init_globals
 */
static void php_sic_init_globals(zend_sic_globals *sic_globals)
{
	sic_globals->enabled = 0;
	sic_globals->shard_num = 2;
	sic_globals->shard_size = 2048;
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(sic)
{
	ZEND_INIT_MODULE_GLOBALS(sic, php_sic_init_globals, NULL);
	REGISTER_INI_ENTRIES();

	if (SIC_G(enabled)) {
		if (0 > sic_init(&si_cache, SIC_G(shard_num), SIC_G(shard_size))) {
			return FAILURE;
		}
	}
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(sic)
{
	if (SIC_G(enabled)) {
		sic_free(&si_cache);
	}
	UNREGISTER_INI_ENTRIES();
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(sic)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "sic support", "enabled");

	if (si_cache.shard_num > 0) {
		php_info_print_table_row(2, "SIC cache initialized", "yes");

		zend_long i;
		size_t total_size = 0;
		size_t total_unused_size = 0;
		size_t total_used_cnt = 0;
		size_t total_used_size = 0;
		size_t total_free_cnt = 0;
		size_t total_free_size = 0;
		size_t total_frag_err_cnt = 0;
		size_t total_oom_err_cnt = 0;

		for (i = 0; i < si_cache.shard_num; i++) {

			sic_shard_header_t *header = (sic_shard_header_t *)si_cache.shards[i].shm;

			total_size += header->size;
			total_unused_size += SHARD_UNUSED_SIZE(header);
			total_used_cnt += header->used.cnt;
			total_used_size += header->used.data_size;
			total_free_cnt += header->free.cnt;
			total_free_size += header->free.data_size;
			total_frag_err_cnt += header->errors.frag_err_cnt;
			total_oom_err_cnt += header->errors.oom_err_cnt;
		}

		char buf[64];
		sprintf(buf, "%zu bytes", total_size);
		php_info_print_table_row(2, "Total cache size", buf);
		sprintf(buf, "%zu bytes", total_unused_size);
		php_info_print_table_row(2, "Total unused size", buf);
		sprintf(buf, "%zu", total_used_cnt);
		php_info_print_table_row(2, "Total cache items", buf);
		sprintf(buf, "%zu bytes", total_used_size);
		php_info_print_table_row(2, "Total used items size", buf);
		sprintf(buf, "%zu", total_free_cnt);
		php_info_print_table_row(2, "Total freed items", buf);
		sprintf(buf, "%zu bytes", total_free_size);
		php_info_print_table_row(2, "Total freed items size", buf);
		sprintf(buf, "%zu", total_frag_err_cnt);
		php_info_print_table_row(2, "Fragmentation errors", buf);
		sprintf(buf, "%zu", total_oom_err_cnt);
		php_info_print_table_row(2, "OOM errors", buf);
	} else {
		php_info_print_table_row(2, "SIC cache initialized", "no");
	}
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES();
}
/* }}} */

/* {{{ sic_module_entry
 */
zend_module_entry sic_module_entry = {
	STANDARD_MODULE_HEADER,
	"sic",
	ext_functions,
	PHP_MINIT(sic),
	PHP_MSHUTDOWN(sic),
	NULL,
	NULL,
	PHP_MINFO(sic),
	PHP_SIC_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_SIC
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(sic)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
