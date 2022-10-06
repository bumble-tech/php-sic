--TEST--
sic_gc() basic tests with wrong params for PHP >=8.0
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

var_dump(print_type_exception(function () { return sic_gc(array()); }));
var_dump(print_type_exception(function () { return sic_gc(""); }));

?>
--EXPECTF--
sic_gc() expects exactly 0 arguments, 1 given
NULL
sic_gc() expects exactly 0 arguments, 1 given
NULL
