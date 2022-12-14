--TEST--
sic_gc() basic tests 
--INI--
sic.enabled=1
sic.shard_num=1
sic.shard_size=1k
--FILE--
<?php

var_dump(sic_gc());

var_dump(sic_set("test", 1));
var_dump(sic_get("test"));
var_dump(sic_del("test"));
var_dump(sic_get("test"));
var_dump(sic_gc());
var_dump(sic_get("test"));

var_dump(sic_set("test", 1));
var_dump(sic_del("test"));
var_dump(sic_set("test", 1));
var_dump(sic_get("test"));
var_dump(sic_gc());
var_dump(sic_get("test"));

?>
--EXPECTF--
bool(true)
bool(true)
int(1)
bool(true)
bool(false)
bool(true)
bool(false)
bool(true)
bool(true)
bool(true)
int(1)
bool(true)
int(1)
