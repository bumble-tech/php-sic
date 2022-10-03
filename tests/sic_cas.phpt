--TEST--
sic_cas() basic tests 
--INI--
sic.enabled=1
sic.shard_num=1
sic.shard_size=1k
--FILE--
<?php

var_dump(sic_cas());
var_dump(sic_cas(array()));
var_dump(sic_cas(""));
var_dump(sic_cas("test"));
var_dump(sic_cas("test", 1));
var_dump(sic_set("test", 1));
var_dump(sic_cas("test", 1, 2));
var_dump(sic_get("test"));

var_dump(sic_cas("test", 1, -1));
var_dump(sic_cas("test", 1, new StdClass));
var_dump(sic_set("", 1));
var_dump(sic_get(""));
var_dump(sic_cas("", 1, PHP_INT_MAX));
var_dump(sic_get(""));
var_dump(sic_cas("", PHP_INT_MAX, 1));
var_dump(sic_get(""));
var_dump(sic_set("", []));
var_dump(sic_cas("", array(), []));
var_dump(sic_get(""));

$key = str_repeat("0123456789", 50);
var_dump(sic_set($key, 1));
var_dump(sic_cas($key, 1, 2));
var_dump(sic_get($key));

$key = str_repeat("0123456789", 100);
var_dump(sic_cas($key, 2, 1));
var_dump(sic_get($key));
--EXPECTF--
Warning: sic_cas() expects exactly 3 parameters, 0 given in %s on line %d
NULL

Warning: sic_cas() expects exactly 3 parameters, 1 given in %s on line %d
NULL

Warning: sic_cas() expects exactly 3 parameters, 1 given in %s on line %d
NULL

Warning: sic_cas() expects exactly 3 parameters, 1 given in %s on line %d
NULL

Warning: sic_cas() expects exactly 3 parameters, 2 given in %s on line %d
NULL
bool(true)
bool(true)
int(2)
bool(false)

Warning: sic_cas() expects parameter 3 to be integer, object given in %s on line %d
NULL
bool(true)
int(1)
bool(true)
int(9223372036854775807)
bool(true)
int(1)

Warning: sic_set() expects parameter 2 to be integer, array given in %s on line %d
NULL

Warning: sic_cas() expects parameter 2 to be integer, array given in %s on line %d
NULL
int(1)
bool(true)
bool(true)
int(2)
bool(false)
bool(false)
