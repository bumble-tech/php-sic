--TEST--
sic_dec() basic tests with wrong params for PHP >=8.0
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

var_dump(print_type_exception(function () { return sic_dec(); }));
var_dump(print_type_exception(function () { return sic_dec(new stdclass); }));
var_dump(print_type_exception(function () { return sic_dec([]); }));

--EXPECTF--
sic_dec() expects at least 1 argument, 0 given
NULL
sic_dec(): Argument #1 ($key) must be of type string, stdClass given
NULL
sic_dec(): Argument #1 ($key) must be of type string, array given
NULL
