#include "suite.h"

#include "cebus/alloc.h"
#include "cebus/collection/array.h"

const uint64_t test_digits[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
const size_t test_digits_len = sizeof(test_digits) / sizeof(*test_digits);

#define DIGIT_ENTRY_DELETED (uint64_t) -1

typedef struct digit
{
    uint64_t value;
} digit;

typedef struct digit_entry
{
    digit* digit;
} digit_entry;

MunitResult should_init_with_initial_capacity(const MunitParameter params[], void* data)
{
    cb_array array;
    cb_array_init_with_capacity(&array, 8, sizeof(uint64_t));

    munit_assert_size(cb_array_size(&array), ==, 0);

    cb_array_free(&array, NULL, NULL);
    return MUNIT_OK;
}

MunitResult should_push(const MunitParameter params[], void* data)
{
    cb_array array;
    cb_array_init_with_capacity(&array, test_digits_len, sizeof(*test_digits));

    {
        size_t i;
        for (i = 0; i < test_digits_len; ++i) {
            uint64_t* digit = cb_array_push(&array);
            *digit = test_digits[i];
        }
    }

    munit_assert_size(cb_array_size(&array), ==, test_digits_len);

    cb_array_free(&array, NULL, NULL);
    return MUNIT_OK;
}

MunitResult should_push_and_grow_as_needed(const MunitParameter params[], void* data)
{
    cb_array array;
    cb_array_init_with_capacity(&array, test_digits_len, sizeof(*test_digits));

    {
        size_t i;
        for (i = 0; i < test_digits_len; ++i) {
            uint64_t* digit = cb_array_push(&array);
            *digit = test_digits[i];
        }
    }

    {
        size_t i;
        for (i = 0; i < test_digits_len; ++i) {
            uint64_t* digit = cb_array_push(&array);
            *digit = test_digits[i] * 2;
        }
    }

    munit_assert_size(cb_array_size(&array), ==, test_digits_len * 2);

    cb_array_free(&array, NULL, NULL);
    return MUNIT_OK;
}

MunitResult should_get(const MunitParameter params[], void* data)
{
    cb_array array;
    cb_array_init_with_capacity(&array, test_digits_len, sizeof(*test_digits));

    {
        size_t i;
        for (i = 0; i < test_digits_len; ++i) {
            uint64_t* digit = cb_array_push(&array);
            *digit = test_digits[i];
        }
    }

    munit_assert_size(cb_array_size(&array), ==, test_digits_len);

    {
        size_t i;
        for (i = 0; i < test_digits_len; ++i) {
            const uint64_t* digit = (const uint64_t *) cb_array_get(&array, i);
            munit_assert_ptr_not_null(digit);
            munit_assert_uint64(*digit, ==, test_digits[i]);
        }
    }

    munit_assert_ptr_null(cb_array_get(&array, test_digits_len * 4));

    cb_array_free(&array, NULL, NULL);
    return MUNIT_OK;
}

MunitResult should_iterate(const MunitParameter params[], void* data)
{
    cb_array array;
    cb_array_init_with_capacity(&array, test_digits_len, sizeof(*test_digits));

    {
        size_t i;
        for (i = 0; i < test_digits_len; ++i) {
            uint64_t* digit = cb_array_push(&array);
            *digit = test_digits[i];
        }
    }

    munit_assert_size(cb_array_size(&array), ==, test_digits_len);

    {
        cb_array_iterator iter = cb_array_iter(&array);
        size_t index = 0;
        while (cb_array_iter_has_next(CB_ARRAY_ITER(iter)) == cebus_true)
        {
            const uint64_t *digit = (const uint64_t *) cb_array_iter_get(iter);
            munit_assert_uint64(*digit, ==, test_digits[index]);

            ++index;
            cb_array_iter_move_next(CB_ARRAY_ITER(iter));
        }

        munit_assert_size(index, ==, cb_array_size(&array));
    }

    return MUNIT_OK;
}

MunitResult should_clear(const MunitParameter params[], void* data)
{
    cb_array array;
    cb_array_init_with_capacity(&array, test_digits_len, sizeof(*test_digits));

    {
        size_t i;
        for (i = 0; i < test_digits_len; ++i) {
            uint64_t* digit = cb_array_push(&array);
            *digit = test_digits[i];
        }
    }

    munit_assert_size(cb_array_size(&array), ==, test_digits_len);

    cb_array_clear(&array, NULL, NULL);

    munit_assert_size(cb_array_size(&array), ==, 0);

    return MUNIT_OK;
}

static void digit_entry_destroy(void* element, void* user)
{
    digit_entry* entry = (digit_entry *) element;
    entry->digit->value = DIGIT_ENTRY_DELETED;
}

MunitResult should_clear_and_call_destructor_on_entry(const MunitParameter params[], void* data)
{
    cb_array array;
    cb_array_init_with_capacity(&array, test_digits_len, sizeof(digit_entry));

    digit *digits = cb_new(digit, test_digits_len);
    {
        size_t i;
        for (i = 0; i < test_digits_len; ++i) {
            digit* digit = digits + i;
            digit->value = test_digits[i];

            digit_entry* entry = cb_array_push(&array);
            entry->digit = digit;
        }
    }

    munit_assert_size(cb_array_size(&array), ==, test_digits_len);
    cb_array_clear(&array, digit_entry_destroy, NULL);
    munit_assert_size(cb_array_size(&array), ==, 0);

    {
        size_t i;
        for (i = 0; i < test_digits_len; ++i) {
            digit *digit = digits + i;
            munit_assert_uint64(digit->value, ==, DIGIT_ENTRY_DELETED);
        }
    }

    free(digits);
    return MUNIT_OK;
}

MunitResult should_copy(const MunitParameter params[], void* data)
{
    cb_array array;
    cb_array array_copy;

    cb_array_init_with_capacity(&array, test_digits_len, sizeof(*test_digits));

    {
        size_t i;
        for (i = 0; i < test_digits_len; ++i) {
            uint64_t* digit = cb_array_push(&array);
            *digit = test_digits[i];
        }
    }

    munit_assert_size(cb_array_size(&array), ==, test_digits_len);

    cb_array_copy(&array_copy, &array, NULL, NULL);
    {
        size_t i;
        for (i = 0; i < test_digits_len; ++i) {
            uint64_t* digit = (uint64_t *) cb_array_get(&array_copy, i);
            munit_assert_ptr_not_null(digit);
            munit_assert_uint64(*digit, ==, test_digits[i]);
        }
    }

    cb_array_free(&array, NULL, NULL);
    cb_array_free(&array_copy, NULL, NULL);

    return MUNIT_OK;
}

CEBUS_DECLARE_TEST_SUITE(
    "array_tests",

    CEBUS_TEST(should_init_with_initial_capacity),
    CEBUS_TEST(should_push),
    CEBUS_TEST(should_push_and_grow_as_needed),
    CEBUS_TEST(should_get),
    CEBUS_TEST(should_iterate),
    CEBUS_TEST(should_clear),
    CEBUS_TEST(should_clear_and_call_destructor_on_entry),
    CEBUS_TEST(should_copy)
)
