#include "munit.h"

#include "cebus/binding_key.h"

MunitResult should_create_binding_key_from_fragments(const  MunitParameter params[], void* data)
{
    const char* fragments[] = { "A", "B", "160" };
    binding_key *key = binding_key_new(fragments, 3);

    munit_assert_ullong(binding_key_fragment_count(key), ==, 3);
    munit_assert_string_equal(binding_key_get_fragment(key, 0).value, "A");
    munit_assert_string_equal(binding_key_get_fragment(key, 1).value, "B");
    munit_assert_string_equal(binding_key_get_fragment(key, 2).value, "160");

    munit_assert_ptr_null(binding_key_get_fragment(key, 8).value);

    return MUNIT_OK;
}

MunitTest tests[] = {
    {
        "should_create_binding_key_from_fragments",
        should_create_binding_key_from_fragments,
        NULL,
        NULL,
        MUNIT_TEST_OPTION_NONE,
        NULL
    },

    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite suite = {
    "binding_key_tests",
    tests,
    NULL,
    1,
    MUNIT_SUITE_OPTION_NONE
};

int main(int argc, const char* argv[])
{
    return munit_suite_main(&suite, NULL, argc, argv);
}
