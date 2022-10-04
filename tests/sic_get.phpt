--TEST--
sic_get() basic tests 
--INI--
sic.enabled=1
sic.shard_num=1
sic.shard_size=1k
--FILE--
<?php

var_dump(sic_get("test"));

var_dump(sic_set("test", 1));
var_dump(sic_set("test", 1));

var_dump(sic_set("test", PHP_INT_MAX));
var_dump(sic_get("test"));

var_dump(sic_set("test", -PHP_INT_MAX));
var_dump(sic_get("test"));
var_dump(sic_get("test_no_such_key"));

$key = str_repeat("0123456789", 100);
var_dump(sic_get($key));

?>
--EXPECTF--
bool(false)
bool(true)
bool(true)
bool(true)
int(9223372036854775807)
bool(true)
int(-9223372036854775807)
bool(false)
bool(false)
