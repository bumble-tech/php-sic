dnl config.m4 for extension sic

PHP_ARG_ENABLE(sic, whether to enable sic support,
[  --enable-sic           Enable sic support])

if test "$PHP_SIC" != "no"; then
  orig_LIBS="$LIBS"
  LIBS="$LIBS -lpthread"
  AC_TRY_RUN(
  [
#include <sys/types.h>
#include <pthread.h>
#include <stdio.h>
main() {
  pthread_rwlock_t rwlock;
  pthread_rwlockattr_t attr;	

  if(pthread_rwlockattr_init(&attr)) { 
    puts("Unable to initialize pthread attributes (pthread_rwlockattr_init).");
    return -1; 
  }
  if(pthread_rwlockattr_setpshared(&attr, PTHREAD_PROCESS_SHARED)) { 
    puts("Unable to set PTHREAD_PROCESS_SHARED (pthread_rwlockattr_setpshared), your system may not support shared rwlock's.");
    return -1; 
  }	
  if(pthread_rwlock_init(&rwlock, &attr)) { 
    puts("Unable to initialize the rwlock (pthread_rwlock_init).");
    return -1; 
  }
  if(pthread_rwlockattr_destroy(&attr)) { 
    puts("Unable to destroy rwlock attributes (pthread_rwlockattr_destroy).");
    return -1; 
  }
  if(pthread_rwlock_destroy(&rwlock)) {
    puts("Unable to destroy rwlock (pthread_rwlock_destroy).");
    return -1; 
  }
  return 0;
}
  ],
  [ dnl -Success-
    SIC_CFLAGS="-D_GNU_SOURCE -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1"
    PHP_ADD_LIBRARY(pthread)
    PHP_LDFLAGS="$PHP_LDFLAGS -lpthread"
  ],
  [ dnl -Failure-
    AC_MSG_ERROR([It doesn't appear that pthread rwlocks are supported on your system. Find a working system.])
  ],
  [
    SIC_CFLAGS="-D_GNU_SOURCE -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1"
    PHP_ADD_LIBRARY(pthread)
    PHP_LDFLAGS="$PHP_LDFLAGS -lpthread"
  ])
  LIBS="$orig_LIBS"

  PHP_NEW_EXTENSION(sic, sic.c, $ext_shared,, \\$(SIC_CFLAGS))
fi
