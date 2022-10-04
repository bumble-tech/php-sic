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

#ifndef PHP_SIC_H
#define PHP_SIC_H

extern zend_module_entry sic_module_entry;
#define phpext_sic_ptr &sic_module_entry

#define PHP_SIC_VERSION "0.1.0" /* Replace with version number for your extension */

#ifdef PHP_WIN32
#	define PHP_SIC_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_SIC_API __attribute__ ((visibility("default")))
#else
#	define PHP_SIC_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

ZEND_BEGIN_MODULE_GLOBALS(sic)
	zend_bool  enabled;
	zend_long  shard_num;
	zend_long  shard_size;
ZEND_END_MODULE_GLOBALS(sic)

#define SIC_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(sic, v)

#if defined(ZTS) && defined(COMPILE_DL_SIC)
ZEND_TSRMLS_CACHE_EXTERN()
#endif

#endif	/* PHP_SIC_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
