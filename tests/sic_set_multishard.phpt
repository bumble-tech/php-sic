--TEST--
sic_set() with multiple shards
--INI--
sic.enabled=1
sic.shard_num=2
sic.shard_size=1k
--FILE--
<?php

for ($i = 0; $i < 27; $i++) {
	var_dump(sic_set("test$i", $i));
}

for ($i = 0; $i < 27; $i++) {
	var_dump(sic_get("test$i"));
}

?>
--EXPECTF--
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(false)
int(0)
int(1)
int(2)
int(3)
int(4)
int(5)
int(6)
int(7)
int(8)
int(9)
int(10)
int(11)
int(12)
int(13)
int(14)
int(15)
int(16)
int(17)
int(18)
int(19)
int(20)
int(21)
int(22)
int(23)
int(24)
int(25)
bool(false)
