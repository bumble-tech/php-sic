--TEST--
sic_del() basic tests with wrong params for PHP 7.x
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

var_dump(sic_del());
var_dump(sic_del(array()));
var_dump(sic_del("test", 1));

?>
--EXPECTF--
Warning: sic_del() expects exactly 1 parameter, 0 given in %s on line %d
NULL

Warning: sic_del() expects parameter 1 to be string, array given in %s on line %d
NULL

Warning: sic_del() expects exactly 1 parameter, 2 given in %s on line %d
NULL
