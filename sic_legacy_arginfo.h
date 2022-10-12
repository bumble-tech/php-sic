/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 5c6c4fcc6fb55a2ebcf6703c0fe8613254f96c3d */

ZEND_BEGIN_ARG_INFO_EX(arginfo_sic_set, 0, 0, 2)
	ZEND_ARG_INFO(0, key)
	ZEND_ARG_INFO(0, value)
	ZEND_ARG_INFO(0, ttl)
ZEND_END_ARG_INFO()

#define arginfo_sic_add arginfo_sic_set

ZEND_BEGIN_ARG_INFO_EX(arginfo_sic_del, 0, 0, 1)
	ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

#define arginfo_sic_get arginfo_sic_del

#define arginfo_sic_exists arginfo_sic_del

ZEND_BEGIN_ARG_INFO_EX(arginfo_sic_inc, 0, 0, 1)
	ZEND_ARG_INFO(0, key)
	ZEND_ARG_INFO(0, inc_value)
	ZEND_ARG_INFO(0, ttl)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sic_dec, 0, 0, 1)
	ZEND_ARG_INFO(0, key)
	ZEND_ARG_INFO(0, dec_value)
	ZEND_ARG_INFO(0, ttl)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sic_cas, 0, 0, 3)
	ZEND_ARG_INFO(0, key)
	ZEND_ARG_INFO(0, old_value)
	ZEND_ARG_INFO(0, new_value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sic_gc, 0, 0, 0)
ZEND_END_ARG_INFO()

#define arginfo_sic_info arginfo_sic_gc


ZEND_FUNCTION(sic_set);
ZEND_FUNCTION(sic_add);
ZEND_FUNCTION(sic_del);
ZEND_FUNCTION(sic_get);
ZEND_FUNCTION(sic_exists);
ZEND_FUNCTION(sic_inc);
ZEND_FUNCTION(sic_dec);
ZEND_FUNCTION(sic_cas);
ZEND_FUNCTION(sic_gc);
ZEND_FUNCTION(sic_info);


static const zend_function_entry ext_functions[] = {
	ZEND_FE(sic_set, arginfo_sic_set)
	ZEND_FE(sic_add, arginfo_sic_add)
	ZEND_FE(sic_del, arginfo_sic_del)
	ZEND_FE(sic_get, arginfo_sic_get)
	ZEND_FE(sic_exists, arginfo_sic_exists)
	ZEND_FE(sic_inc, arginfo_sic_inc)
	ZEND_FE(sic_dec, arginfo_sic_dec)
	ZEND_FE(sic_cas, arginfo_sic_cas)
	ZEND_FE(sic_gc, arginfo_sic_gc)
	ZEND_FE(sic_info, arginfo_sic_info)
	ZEND_FE_END
};
