--TEST--
sic_cas() basic tests with wrong params for PHP >= 8.0
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

var_dump(print_type_exception(function () { return sic_cas(); }));
var_dump(print_type_exception(function () { return sic_cas(array()); }));
var_dump(print_type_exception(function () { return sic_cas(""); }));
var_dump(print_type_exception(function () { return sic_cas("test"); }));
var_dump(print_type_exception(function () { return sic_cas("test", 1); }));

var_dump(print_type_exception(function () { return sic_cas("test", 1, new StdClass); }));
var_dump(print_type_exception(function () { return sic_cas("", array(), []); }));

--EXPECTF--
sic_cas() expects exactly 3 arguments, 0 given
NULL
sic_cas() expects exactly 3 arguments, 1 given
NULL
sic_cas() expects exactly 3 arguments, 1 given
NULL
sic_cas() expects exactly 3 arguments, 1 given
NULL
sic_cas() expects exactly 3 arguments, 2 given
NULL
sic_cas(): Argument #3 ($new_value) must be of type int, stdClass given
NULL
sic_cas(): Argument #2 ($old_value) must be of type int, array given
NULL
