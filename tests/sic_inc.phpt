--TEST--
sic_inc() basic tests 
--INI--
sic.enabled=1
sic.shard_num=1
sic.shard_size=1k
--FILE--
<?php

var_dump(sic_inc());
var_dump(sic_inc("test", 1));
var_dump(sic_get("test"));
var_dump(sic_inc("test"));
var_dump(sic_get("test"));
var_dump(sic_inc(new stdclass));
var_dump(sic_get(new stdclass));
var_dump(sic_inc([]));
var_dump(sic_get([]));

var_dump(sic_set("test", 1));
var_dump(sic_set("test", 1));

var_dump(sic_set("test", PHP_INT_MAX));
var_dump(sic_inc("test"));
var_dump(sic_get("test"));

var_dump(sic_set("test", -PHP_INT_MAX));
var_dump(sic_inc("test"));
var_dump(sic_get("test"));
var_dump(sic_inc("test_no_such_key", 10));
var_dump(sic_get("test_no_such_key"));

$key = str_repeat("0123456789", 100);
var_dump(sic_inc($key));
var_dump(sic_get($key));
--EXPECTF--
Warning: sic_inc() expects at least 1 parameter, 0 given in %s on line %d
NULL
int(1)
int(1)
int(2)
int(2)

Warning: sic_inc() expects parameter 1 to be string, object given in %s on line %d
NULL

Warning: sic_get() expects parameter 1 to be string, object given in %s on line %d
NULL

Warning: sic_inc() expects parameter 1 to be string, array given in %s on line %d
NULL

Warning: sic_get() expects parameter 1 to be string, array given in %s on line %d
NULL
bool(true)
bool(true)
bool(true)
int(-9223372036854775808)
int(-9223372036854775808)
bool(true)
int(-9223372036854775806)
int(-9223372036854775806)
int(10)
int(10)
bool(false)
bool(false)
