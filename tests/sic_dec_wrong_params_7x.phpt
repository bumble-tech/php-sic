--TEST--
sic_dec() basic tests with wrong params for PHP 7.x
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

var_dump(sic_dec());
var_dump(sic_dec(new stdclass));
var_dump(sic_dec([]));

--EXPECTF--
Warning: sic_dec() expects at least 1 parameter, 0 given in %s on line %d
NULL

Warning: sic_dec() expects parameter 1 to be string, object given in %s on line %d
NULL

Warning: sic_dec() expects parameter 1 to be string, array given in %s on line %d
NULL
