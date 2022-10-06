--TEST--
sic_set() basic tests with wrong params for PHP >=8.0
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

var_dump(print_type_exception(function () { return sic_set(); }));
var_dump(print_type_exception(function () { return sic_set(array()); }));
var_dump(print_type_exception(function () { return sic_set(""); }));
var_dump(print_type_exception(function () { return sic_set("test"); }));
var_dump(print_type_exception(function () { return sic_set("test", 1, 1, 1); }));
var_dump(print_type_exception(function () { return sic_set("test", 1, new StdClass); }));
var_dump(print_type_exception(function () { return sic_set("", array()); }));

?>
--EXPECTF--
sic_set() expects at least 2 arguments, 0 given
NULL
sic_set() expects at least 2 arguments, 1 given
NULL
sic_set() expects at least 2 arguments, 1 given
NULL
sic_set() expects at least 2 arguments, 1 given
NULL
sic_set() expects at most 3 arguments, 4 given
NULL
sic_set(): Argument #3 ($ttl) must be of type int, stdClass given
NULL
sic_set(): Argument #2 ($value) must be of type int, array given
NULL
