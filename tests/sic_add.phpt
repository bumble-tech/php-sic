--TEST--
sic_add() basic tests 
--INI--
sic.enabled=1
sic.shard_num=1
sic.shard_size=1k
--FILE--
<?php

var_dump(sic_add());
var_dump(sic_add(array()));
var_dump(sic_add(""));
var_dump(sic_add("test"));
var_dump(sic_add("test", 1));
var_dump(sic_add("test", 1));
var_dump(sic_add("test", 1, 1));
var_dump(sic_add("test", 1, 1, 1));
var_dump(sic_add("test", 1, new StdClass));
var_dump(sic_add("", 1));
var_dump(sic_add("", array()));

$key = str_repeat("0123456789", 50);
var_dump(sic_add($key, 1));

$key = str_repeat("0123456789", 100);
var_dump(sic_add($key, 1));
--EXPECTF--
Warning: sic_add() expects at least 2 parameters, 0 given in %s on line %d
NULL

Warning: sic_add() expects at least 2 parameters, 1 given in %s on line %d
NULL

Warning: sic_add() expects at least 2 parameters, 1 given in %s on line %d
NULL

Warning: sic_add() expects at least 2 parameters, 1 given in %s on line %d
NULL
bool(true)
bool(false)
bool(false)

Warning: sic_add() expects at most 3 parameters, 4 given in %s on line %d
NULL

Warning: sic_add() expects parameter 3 to be integer, object given in %s on line %d
NULL
bool(true)

Warning: sic_add() expects parameter 2 to be integer, array given in %s on line %d
NULL
bool(true)
bool(false)
