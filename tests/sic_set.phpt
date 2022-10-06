--TEST--
sic_set() basic tests 
--INI--
sic.enabled=1
sic.shard_num=1
sic.shard_size=1k
--FILE--
<?php

var_dump(sic_set("test", 1));
var_dump(sic_set("test", 1, 1));
var_dump(sic_set("", 1));

$key = str_repeat("0123456789", 50);
var_dump(sic_set($key, 1));

$key = str_repeat("0123456789", 100);
var_dump(sic_set($key, 1));

?>
--EXPECTF--
bool(true)
bool(true)
bool(true)
bool(true)
bool(false)
