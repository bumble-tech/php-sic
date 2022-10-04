--TEST--
sic_exists() basic tests with wrong params for PHP 7.x
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

var_dump(sic_exists());
var_dump(sic_exists("test", 1));
var_dump(sic_exists(new stdclass));
var_dump(sic_exists([]));

--EXPECTF--
Warning: sic_exists() expects exactly 1 parameter, 0 given in %s on line %d
NULL

Warning: sic_exists() expects exactly 1 parameter, 2 given in %s on line %d
NULL

Warning: sic_exists() expects parameter 1 to be string, object given in %s on line %d
NULL

Warning: sic_exists() expects parameter 1 to be string, array given in %s on line %d
NULL
