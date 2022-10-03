--TEST--
sic_gc() list rebuild 
--INI--
sic.enabled=1
sic.shard_num=1
sic.shard_size=1k
--FILE--
<?php

$key = str_repeat("test", 30);
for ($i = 0; $i < 10; $i++) {
	var_dump(sic_set($key.$i, $i));
}

var_dump(sic_del($key."2"));
var_dump(sic_del($key."1"));

var_dump(sic_set($key.$key.$key, 10)); //GC here

var_dump(sic_get($key.$key));
var_dump(sic_get($key."0"));
var_dump(sic_get($key."3"));
var_dump(sic_get($key."4"));

var_dump(sic_info());


?>
--EXPECTF--
bool(true)
bool(true)
bool(true)
bool(true)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(true)
bool(true)
bool(true)
bool(false)
int(0)
int(3)
bool(false)
array(1) {
  [0]=>
  array(8) {
    ["size"]=>
    int(888)
    ["unused_size"]=>
    int(96)
    ["used_cnt"]=>
    int(3)
    ["used_data_size"]=>
    int(792)
    ["free_cnt"]=>
    int(0)
    ["free_data_size"]=>
    int(0)
    ["frag_err_cnt"]=>
    int(0)
    ["oom_err_cnt"]=>
    int(6)
  }
}
