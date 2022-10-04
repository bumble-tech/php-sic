--TEST--
sic_gc() basic tests with wrong params for PHP 7.x
--SKIPIF--
<?php
if (PHP_VERSION_ID >= 80000) die('skip Only for PHP < 8.0');
?>
--INI--
sic.enabled=1
sic.shard_num=1
sic.shard_size=1k
--FILE--
<?php

var_dump(sic_gc(array()));
var_dump(sic_gc(""));

?>
--EXPECTF--
Warning: sic_gc() expects exactly 0 parameters, 1 given in %s on line %d
NULL

Warning: sic_gc() expects exactly 0 parameters, 1 given in %s on line %d
NULL
