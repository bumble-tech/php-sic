--TEST--
sic_set() with TTL 
--INI--
sic.enabled=1
sic.shard_num=1
sic.shard_size=1k
--FILE--
<?php

var_dump(sic_set("test", 1, 1));
var_dump(sic_get("test"));
sleep(2);
var_dump(sic_get("test"));
var_dump(sic_add("test", 1));
var_dump(sic_get("test"));

?>
--EXPECTF--
bool(true)
int(1)
bool(false)
bool(true)
int(1)
