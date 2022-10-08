<?php

/**
 * @generate-class-entries
 * @generate-legacy-arginfo
 */

function sic_set(string $key, int $value, int $ttl = 0): bool {}

function sic_add(string $key, int $value, int $ttl = 0): bool {}

function sic_del(string $key): bool {}

function sic_get(string $key): int|false {}

function sic_exists(string $key): bool {}

function sic_inc(string $key, int $inc_value = 1, int $ttl = 0): int|false {}

function sic_dec(string $key, int $dec_value = 1, int $ttl = 0): int|false {}

function sic_cas(string $key, int $old_value, int $new_value): bool {}

function sic_gc(): bool {}

/** @refcount 1 */
function sic_info(): array|false {}

