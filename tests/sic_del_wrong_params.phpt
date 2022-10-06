--TEST--
sic_del() basic tests with wrong params for PHP >=8.0
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

var_dump(print_type_exception(function () { return sic_del(); }));
var_dump(print_type_exception(function () { return sic_del(array()); }));
var_dump(print_type_exception(function () { return sic_del("test", 1); }));

?>
--EXPECTF--
sic_del() expects exactly 1 argument, 0 given
NULL
sic_del(): Argument #1 ($key) must be of type string, array given
NULL
sic_del() expects exactly 1 argument, 2 given
NULL
