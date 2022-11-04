--TEST--
sic_cas() basic tests with wrong params for PHP 7.3-7.4
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

var_dump(sic_cas());
var_dump(sic_cas(array()));
var_dump(sic_cas(""));
var_dump(sic_cas("test"));
var_dump(sic_cas("test", 1));

var_dump(sic_cas("test", 1, new StdClass));
var_dump(sic_cas("", array(), []));

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

Warning: sic_cas() expects parameter 3 to be int, object given in %s on line %d
NULL

Warning: sic_cas() expects parameter 2 to be int, array given in %s on line %d
NULL
