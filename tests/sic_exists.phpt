--TEST--
sic_exists() basic tests 
--INI--
sic.enabled=1
sic.shard_num=1
sic.shard_size=1k
--FILE--
<?php

var_dump(sic_exists("test"));

var_dump(sic_set("test", 1));
var_dump(sic_set("test", 1));

var_dump(sic_set("test", PHP_INT_MAX));
var_dump(sic_exists("test"));

var_dump(sic_set("test", -PHP_INT_MAX));
var_dump(sic_exists("test"));
var_dump(sic_exists("test_no_such_key"));

$key = str_repeat("0123456789", 100);
var_dump(sic_exists($key));
--EXPECTF--
bool(false)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(false)
bool(false)
