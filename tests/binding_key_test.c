#include "suite.h"

#include "cebus/binding_key.h"
#include "test_routable_event.pb-c.h"
#include "test_routable_event.pb-cb.h"

#include <stdio.h>

MunitResult should_create_binding_key_from_fragments(const  MunitParameter params[], void* data)
{
    const char* fragments[] = { "A", "B", "160" };
    cb_binding_key key = cb_binding_key_from_fragments(fragments, 3);

    munit_assert_ullong(cb_binding_key_fragment_count(key), ==, 3);
    munit_assert_string_equal(cb_binding_key_get_fragment(key, 0).value, "A");
    munit_assert_string_equal(cb_binding_key_get_fragment(key, 1).value, "B");
    munit_assert_string_equal(cb_binding_key_get_fragment(key, 2).value, "160");

    munit_assert_ptr_null(cb_binding_key_get_fragment(key, 8).value);

    return MUNIT_OK;
}

MunitResult should_string_binding_key(const MunitParameter params[], void* data)
{
    char* single_fragment[] = { "hello" };
    char* multiple_fragments[] = { "hello", "987661", "0", "*" };

    struct test_case
    {
        const char* name;

        char** parts;
        size_t count;

        const char* expected;
    } test_cases[] = {
    {
        "null",

        NULL,
        0,

        CB_BINDING_KEY_EMPTY
    },
    {
        "single_fragment",

        single_fragment,
        1,

        "hello"
    },
    {
        "multiple_fragments",

        multiple_fragments,
        sizeof(multiple_fragments) / sizeof(*multiple_fragments),

        "hello.987661.0.*"
    },
    };

    const size_t test_case_n = sizeof(test_cases) / sizeof(*test_cases);
    size_t i;

    for (i = 0; i< test_case_n; ++i)
    {
        const struct test_case* test_case = &test_cases[i];
        cb_binding_key key;

        printf("/%s\n", test_case->name);

        key = cb_binding_key_from_fragments_raw(test_case->parts, test_case->count);

        char* key_str = cb_binding_key_str(key);
        munit_assert_string_equal(key_str, test_case->expected);

        free(key_str);
    }

    return MUNIT_OK;
}

MunitResult should_create_binding_key_from_proto_binding(const MunitParameter params[], void* data)
{
    TestRoutableEventBinding binding = TEST_ROUTABLE_EVENT_BINDING__INIT;
    cb_binding_key key;
    char* str;

    binding.str_field = "test_binding";
    binding.int_field = "45";
    binding.bool_field = "0";

    key = test_routable_event_binding__key(&binding);
    str = cb_binding_key_str(key);

    munit_assert_ullong(cb_binding_key_fragment_count(key), ==, 3);
    munit_assert_string_equal(cb_binding_key_get_fragment(key, 0).value, "test_binding");
    munit_assert_string_equal(cb_binding_key_get_fragment(key, 1).value, "0");
    munit_assert_string_equal(cb_binding_key_get_fragment(key, 2).value, "45");
    munit_assert_string_equal(str, "test_binding.0.45");

    free(str);
    return MUNIT_OK;
}

MunitResult should_create_partial_binding_key_from_proto_binding(const MunitParameter params[], void* data)
{
    TestRoutableEventBinding binding = TEST_ROUTABLE_EVENT_BINDING__INIT;
    cb_binding_key key;
    char* str;

    binding.str_field = "test_binding";

    key = test_routable_event_binding__key(&binding);
    str = cb_binding_key_str(key);

    munit_assert_ullong(cb_binding_key_fragment_count(key), ==, 3);
    munit_assert_string_equal(cb_binding_key_get_fragment(key, 0).value, "test_binding");
    munit_assert_string_equal(cb_binding_key_get_fragment(key, 1).value, CB_BINDING_KEY_ALL);
    munit_assert_string_equal(cb_binding_key_get_fragment(key, 2).value, CB_BINDING_KEY_ALL);
    munit_assert_string_equal(str, "test_binding.*.*");

    free(str);
    return MUNIT_OK;
}

MunitResult should_create_binding_key_from_proto(const MunitParameter params[], void* data)
{
    TestRoutableEvent event = TEST_ROUTABLE_EVENT__INIT;
    cb_binding_key key;
    char* str;

    event.str_field = "test_value";
    event.int_field = 8765181;
    event.bool_field = 1;

    key = test_routable_event__key(&event);
    str = cb_binding_key_str(key);

    munit_assert_ullong(cb_binding_key_fragment_count(key), ==, 3);
    munit_assert_string_equal(cb_binding_key_get_fragment(key, 0).value, "test_value");
    munit_assert_string_equal(cb_binding_key_get_fragment(key, 1).value, "1");
    munit_assert_string_equal(cb_binding_key_get_fragment(key, 2).value, "8765181");
    munit_assert_string_equal(str, "test_value.1.8765181");

    free(str);
    return MUNIT_OK;
}

MunitResult should_create_binding_key_from_str(const MunitParameter params[], void* data)
{
    cb_binding_key key;

    key = cb_binding_key_from_str("my.message.1234");
    munit_assert_ullong(cb_binding_key_fragment_count(key), ==, 3);
    munit_assert_string_equal(cb_binding_key_get_fragment(key, 0).value, "my");
    munit_assert_string_equal(cb_binding_key_get_fragment(key, 1).value, "message");
    munit_assert_string_equal(cb_binding_key_get_fragment(key, 2).value, "1234");
    cb_binding_key_free(&key);

    key = cb_binding_key_from_str("*");
    munit_assert_ullong(cb_binding_key_fragment_count(key), ==, 1);
    munit_assert_string_equal(cb_binding_key_get_fragment(key, 0).value, "*");
    cb_binding_key_free(&key);

    return MUNIT_OK;
}

CEBUS_DECLARE_TEST_SUITE(
    binding_key_tests,

    CEBUS_TEST(should_create_binding_key_from_fragments),
    CEBUS_TEST(should_string_binding_key),
    CEBUS_TEST(should_create_binding_key_from_proto_binding),
    CEBUS_TEST(should_create_partial_binding_key_from_proto_binding),
    CEBUS_TEST(should_create_binding_key_from_proto),
    CEBUS_TEST(should_create_binding_key_from_str)
)
