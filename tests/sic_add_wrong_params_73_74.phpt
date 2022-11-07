--TEST--
sic_add() basic tests with wrong params for PHP 7.3-7.4
--SKIPIF--
<?php
if (PHP_VERSION_ID >= 80000 || PHP_VERSION_ID < 70300) die('skip Only for PHP 7.3-7.4');
?>
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
var_dump(sic_add("test", 1, 1, 1));
var_dump(sic_add("test", 1, new StdClass));
var_dump(sic_add("", array()));

--EXPECTF--
Warning: sic_add() expects at least 2 parameters, 0 given in %s on line %d
NULL

Warning: sic_add() expects at least 2 parameters, 1 given in %s on line %d
NULL

Warning: sic_add() expects at least 2 parameters, 1 given in %s on line %d
NULL

Warning: sic_add() expects at least 2 parameters, 1 given in %s on line %d
NULL

Warning: sic_add() expects at most 3 parameters, 4 given in %s on line %d
NULL

Warning: sic_add() expects parameter 3 to be int, object given in %s on line %d
NULL

Warning: sic_add() expects parameter 2 to be int, array given in %s on line %d
NULL
