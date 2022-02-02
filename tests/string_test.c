#include "suite.h"

#include "cebus/string.h"

MunitResult should_replace_string(const MunitParameter params[], void* data)
{
    char out[255];
    const char* str = "hello world";
    cb_str_replace(str, "world", "sailor", out, sizeof(out));

    munit_assert_string_equal(out, "hello sailor");

    return MUNIT_OK;
}

MunitResult should_replace_string_size(const MunitParameter params[], void* data)
{
    char out[10];
    const char* str = "hello world";
    cb_str_replace(str, "world", "sailor", out, sizeof(out));

    munit_assert_string_equal(out, "hello sai");

    return MUNIT_OK;
}

CEBUS_DECLARE_TEST_SUITE(
    string_tests,

    CEBUS_TEST(should_replace_string),
    CEBUS_TEST(should_replace_string_size)
)
