--TEST--
sic_dec() basic tests 
--INI--
sic.enabled=1
sic.shard_num=1
sic.shard_size=1k
--FILE--
<?php

var_dump(sic_dec("test", 1));
var_dump(sic_get("test"));
var_dump(sic_dec("test"));
var_dump(sic_get("test"));

var_dump(sic_set("test", 1));
var_dump(sic_set("test", 1));

var_dump(sic_set("test", PHP_INT_MAX));
var_dump(sic_dec("test"));
var_dump(sic_get("test"));

var_dump(sic_set("test", -PHP_INT_MAX));
var_dump(sic_dec("test"));
var_dump(sic_get("test"));
var_dump(sic_dec("test_no_such_key", 10));
var_dump(sic_get("test_no_such_key"));

$key = str_repeat("0123456789", 100);
var_dump(sic_dec($key));
var_dump(sic_get($key));
--EXPECTF--
int(-1)
int(-1)
int(-2)
int(-2)
bool(true)
bool(true)
bool(true)
int(9223372036854775806)
int(9223372036854775806)
bool(true)
int(-9223372036854775808)
int(-9223372036854775808)
int(-10)
int(-10)
bool(false)
bool(false)
