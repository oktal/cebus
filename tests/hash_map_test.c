#include "suite.h"

#include "cebus/alloc.h"
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

#define DIGIT_ENTRY_DELETED (uint64_t) -1

typedef struct digit_entry
{
    uint64_t value;
} digit_entry;

MunitResult should_insert(const MunitParameter params[], void* data)
{
    size_t i;
    cb_hash_map* digits_map = cb_hash_map_new(cb_hash_u64, cb_hash_eq_u64);
    cb_hasher_init(&digits_map->hasher, 0xdead, 0xbeef);

    for (i = 0; i < digits_len; ++i)
    {
        munit_assert_ptr_not_null(cb_hash_insert(digits_map, &digits[i], (char *) digits_str_en[i]));
    }

    munit_assert_ullong(cb_hash_len(digits_map), ==, digits_len);

    for (i = 0; i < digits_len; ++i)
    {
        cb_hash_value_t value = cb_hash_insert(digits_map, &digits[i], (char *) digits_str_fr[i]);
        munit_assert_ptr_not_null(value);
        munit_assert_string_equal(value, digits_str_fr[i]);
    }

    munit_assert_ullong(cb_hash_len(digits_map), ==, digits_len);

    cb_hash_map_free(digits_map, NULL, NULL);
    return MUNIT_OK;
}

MunitResult should_get(const MunitParameter params[], void* data)
{
    size_t i;
    cb_hash_map* digits_map = cb_hash_map_new(cb_hash_u64, cb_hash_eq_u64);
    cb_hasher_init(&digits_map->hasher, 0xdead, 0xbeef);

    for (i = 0; i < digits_len; ++i)
    {
        munit_assert_ptr_not_null(cb_hash_insert(digits_map, &digits[i], (char *) digits_str_en[i]));
    }

    for (i = 0; i < digits_len; ++i)
    {
        cb_hash_value_t value = cb_hash_get(digits_map, &digits[i]);
        munit_assert_ptr_not_null(value);
        munit_assert_string_equal(value, digits_str_en[i]);
    }

    munit_assert_ullong(cb_hash_len(digits_map), ==, digits_len);

    cb_hash_map_free(digits_map, NULL, NULL);
    return MUNIT_OK;
}

MunitResult should_remove(const MunitParameter params[], void* data)
{
    size_t i;
    cb_hash_map* digits_map = cb_hash_map_new(cb_hash_u64, cb_hash_eq_u64);
    cb_hasher_init(&digits_map->hasher, 0xdead, 0xbeef);

    for (i = 0; i < digits_len; ++i)
    {
        munit_assert_ptr_not_null(cb_hash_insert(digits_map, &digits[i], (char *) digits_str_en[i]));
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
    cb_hash_map_free(digits_map, NULL, NULL);

    return MUNIT_OK;
}

static void digit_entry_destroy(cb_hash_key_t key, cb_hash_value_t value, void* user)
{
    digit_entry* entry = (digit_entry *) value;
    entry->value = DIGIT_ENTRY_DELETED;
}

MunitResult should_clear(const MunitParameter params[], void* data)
{
    size_t i;
    cb_hash_map* digits_map = cb_hash_map_new(cb_hash_u64, cb_hash_eq_u64);
    digit_entry* entries = cb_new(digit_entry, digits_len);

    cb_hasher_init(&digits_map->hasher, 0xdead, 0xbeef);
    for (i = 0; i < digits_len; ++i)
    {
        digit_entry* entry = entries + i;
        entry->value = digits[i];
        munit_assert_ptr_not_null(cb_hash_insert(digits_map, &digits[i], entry));
    }

    munit_assert_uint64(cb_hash_len(digits_map), ==, digits_len);
    cb_hash_clear(digits_map, digit_entry_destroy, NULL);
    munit_assert_uint64(cb_hash_len(digits_map), ==, 0);

    for (i = 0; i < digits_len; ++i)
    {
        digit_entry* entry = entries + i;
        munit_assert_int64(entry->value, ==, DIGIT_ENTRY_DELETED);
    }

    cb_hash_map_free(digits_map, NULL, NULL);
    free(entries);

    return MUNIT_OK;
}

CEBUS_DECLARE_TEST_SUITE(
    "hash_map_tests",

    CEBUS_TEST(should_insert),
    CEBUS_TEST(should_get),
    CEBUS_TEST(should_remove),
    CEBUS_TEST(should_clear)
);
