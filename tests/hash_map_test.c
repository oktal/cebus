#include "suite.h"

#include "cebus/collection/hash_map.h"

#include <stdio.h>

static void cb_hash_print(const cb_hash_map* map)
{
    size_t i;
    for (i = 0; i < map->size; ++i) {
        const cb_hash_bucket* bucket = &map->buckets[i];
        printf("entry(%lu) = { .hash = %ull, key = %p, value = %p }\n", i,
                bucket->hash, bucket->key, bucket->value);
    }
}

uint64_t digits[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
const size_t digits_len = sizeof(digits) / sizeof(*digits);

const char* digits_str_en[] = { "zero", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine" };
const char* digits_str_fr[] = { "zero", "un", "deux", "trois", "quatre", "cinq", "six", "sept", "huit", "neuf" };

MunitResult should_insert(const MunitParameter params[], void* data)
{
    size_t i;
    cb_hash_map* digits_map = cb_hash_map_new(cb_hash_u64, cb_hash_eq_u64);
    cb_hasher_init(&digits_map->hasher, 0xdead, 0xbeef);

    for (i = 0; i < digits_len; ++i)
    {
        munit_assert_ptr_null(cb_hash_insert(digits_map, &digits[i], (char *) digits_str_en[i]));
    }

    munit_assert_ullong(cb_hash_len(digits_map), ==, digits_len);

    for (i = 0; i < digits_len; ++i)
    {
        cb_hash_value_t value = cb_hash_insert(digits_map, &digits[i], (char *) digits_str_fr[i]);
        munit_assert_ptr_not_null(value);
        munit_assert_string_equal(value, digits_str_en[i]);
    }

    munit_assert_ullong(cb_hash_len(digits_map), ==, digits_len);

    free(digits_map);
    return MUNIT_OK;
}

MunitResult should_get(const MunitParameter params[], void* data)
{
    size_t i;
    cb_hash_map* digits_map = cb_hash_map_new(cb_hash_u64, cb_hash_eq_u64);
    cb_hasher_init(&digits_map->hasher, 0xdead, 0xbeef);

    for (i = 0; i < digits_len; ++i)
    {
        munit_assert_ptr_null(cb_hash_insert(digits_map, &digits[i], (char *) digits_str_en[i]));
    }

    for (i = 0; i < digits_len; ++i)
    {
        cb_hash_value_t value = cb_hash_get(digits_map, &digits[i]);
        munit_assert_ptr_not_null(value);
        munit_assert_string_equal(value, digits_str_en[i]);
    }

    munit_assert_ullong(cb_hash_len(digits_map), ==, digits_len);

    free(digits_map);
    return MUNIT_OK;
}

MunitResult should_remove(const MunitParameter params[], void* data)
{
    size_t i;
    cb_hash_map* digits_map = cb_hash_map_new(cb_hash_u64, cb_hash_eq_u64);
    cb_hasher_init(&digits_map->hasher, 0xdead, 0xbeef);

    for (i = 0; i < digits_len; ++i)
    {
        munit_assert_ptr_null(cb_hash_insert(digits_map, &digits[i], (char *) digits_str_en[i]));
    }

    for (i = 0; i < digits_len; ++i)
    {
        cb_hash_value_t value = cb_hash_remove(digits_map, &digits[i]);
        munit_assert_ptr_not_null(value);
        munit_assert_string_equal(value, digits_str_en[i]);
    }

    for (i = 0; i < digits_len; ++i)
    {
        munit_assert_ptr_null(cb_hash_get(digits_map, &digits[i]));
    }

    munit_assert_ullong(cb_hash_len(digits_map), ==, 0);

    free(digits_map);
    return MUNIT_OK;
}

CEBUS_DECLARE_TEST_SUITE(
    "hash_map_tests",

    CEBUS_TEST(should_insert),
    CEBUS_TEST(should_get),
    CEBUS_TEST(should_remove)
);
