--TEST--
sic_get() basic tests with wrong params for PHP >=8.0
--SKIPIF--
<?php
if (PHP_VERSION_ID < 80000) die('skip Only for PHP >= 8.0');
?>
--INI--
sic.enabled=1
sic.shard_num=1
sic.shard_size=1k
--FILE--
<?php

require_once 'print_type_exception.inc';

var_dump(print_type_exception(function () { return sic_get(); }));
var_dump(print_type_exception(function () { return sic_get("test", 1); }));
var_dump(print_type_exception(function () { return sic_get(new stdclass); }));
var_dump(print_type_exception(function () { return sic_get([]); }));

?>
--EXPECTF--
sic_get() expects exactly 1 argument, 0 given
NULL
sic_get() expects exactly 1 argument, 2 given
NULL
sic_get(): Argument #1 ($key) must be of type string, stdClass given
NULL
sic_get(): Argument #1 ($key) must be of type string, array given
NULL
