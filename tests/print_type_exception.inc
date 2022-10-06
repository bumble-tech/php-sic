<?php

// convert TypeError exception back to warning to make tests compatible with both PHP 7.x and PHP 8.x
function print_type_exception(\Closure $f)
{
    try {
        return $f();
    } catch (\TypeError $e) {
        echo $e->getMessage(), "\n";
    }

    return null;
}