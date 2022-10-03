--TEST--
sic_set() with TTL 
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
