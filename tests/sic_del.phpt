--TEST--
sic_del() basic tests 
--INI--
sic.enabled=1
sic.shard_num=1
sic.shard_size=1k
--FILE--
<?php

var_dump(sic_del(""));
var_dump(sic_del("test"));

var_dump(sic_set("test", 1));
var_dump(sic_del("test"));
var_dump(sic_del("test"));
var_dump(sic_set("", 1));
var_dump(sic_del(""));
var_dump(sic_del(""));
var_dump(sic_set("test", 1));
var_dump(sic_del("test"));

$key = str_repeat("0123456789", 50);
var_dump(sic_set($key, 1));
var_dump(sic_del($key));

$key = str_repeat("0123456789", 100);
var_dump(sic_set($key, 1));
var_dump(sic_del($key));

?>
--EXPECTF--
bool(false)
bool(false)
bool(true)
bool(true)
bool(false)
bool(true)
bool(true)
bool(false)
bool(true)
bool(true)
bool(true)
bool(true)
bool(false)
bool(false)
