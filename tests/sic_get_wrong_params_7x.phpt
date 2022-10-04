--TEST--
sic_get() basic tests with wrong params for PHP 7.x
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

var_dump(sic_get());
var_dump(sic_get("test", 1));
var_dump(sic_get(new stdclass));
var_dump(sic_get([]));

?>
--EXPECTF--
Warning: sic_get() expects exactly 1 parameter, 0 given in %s on line %d
NULL

Warning: sic_get() expects exactly 1 parameter, 2 given in %s on line %d
NULL

Warning: sic_get() expects parameter 1 to be string, object given in %s on line %d
NULL

Warning: sic_get() expects parameter 1 to be string, array given in %s on line %d
NULL
