--TEST--
sic_add() basic tests with wrong params for PHP >=8.0
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

var_dump(print_type_exception(function () { return sic_add(); }));
var_dump(print_type_exception(function () { return sic_add(array()); }));
var_dump(print_type_exception(function () { return sic_add(""); }));
var_dump(print_type_exception(function () { return sic_add("test"); }));
var_dump(print_type_exception(function () { return sic_add("test", 1, 1, 1); }));
var_dump(print_type_exception(function () { return sic_add("test", 1, new StdClass); }));
var_dump(print_type_exception(function () { return sic_add("", array()); }));

--EXPECTF--
sic_add() expects at least 2 arguments, 0 given
NULL
sic_add() expects at least 2 arguments, 1 given
NULL
sic_add() expects at least 2 arguments, 1 given
NULL
sic_add() expects at least 2 arguments, 1 given
NULL
sic_add() expects at most 3 arguments, 4 given
NULL
sic_add(): Argument #3 ($ttl) must be of type int, stdClass given
NULL
sic_add(): Argument #2 ($value) must be of type int, array given
NULL
