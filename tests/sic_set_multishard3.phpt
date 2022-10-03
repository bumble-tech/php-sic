--TEST--
sic_set() with multiple shards plus gc
--INI--
sic.enabled=1
sic.shard_num=2
sic.shard_size=1k
--FILE--
<?php

echo "1. add large entry \n";

$key1 = str_repeat("test", 200);
var_dump(sic_set($key1, 1));
var_dump(sic_get($key1));

echo "2. add another large entry \n";

$key2 = str_repeat("a0", 201);
var_dump(sic_set($key2, 2));
var_dump(sic_get($key2));

echo "3. try to add smaller entry - failure \n";

$key3 = str_repeat("a0", 50);
var_dump(sic_set($key3, 3));
var_dump(sic_get($key3));

var_dump(sic_del($key1));

echo "4. try to add small entry after deleting the large one - success \n";

$key3 = str_repeat("a0", 50);
var_dump(sic_set($key3, 3));
var_dump(sic_get($key3));

echo "5. try to add another small entry after deleting the large one - failure because of fragmentation \n";

$key4 = str_repeat("a2", 50);
var_dump(sic_set($key4, 4));
var_dump(sic_get($key4));

sic_gc();

echo "6. run gc and try to add another small entry - success \n";

$key4 = str_repeat("a2", 50);
var_dump(sic_set($key4, 4));
var_dump(sic_get($key4));

?>
--EXPECTF--
1. add large entry 
bool(true)
int(1)
2. add another large entry 
bool(true)
int(2)
3. try to add smaller entry - failure 
bool(false)
bool(false)
bool(true)
4. try to add small entry after deleting the large one - success 
bool(true)
int(3)
5. try to add another small entry after deleting the large one - failure because of fragmentation 
bool(false)
bool(false)
6. run gc and try to add another small entry - success 
bool(true)
int(4)
