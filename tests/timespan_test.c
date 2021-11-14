#include "suite.h"

#include "cebus/utils/timespan.h"

MunitResult should_convert_to_secs(const MunitParameter params[], void* data)
{
    munit_assert_uint64(timespan_as_secs(timespan_from_secs(1)), ==, 1);

    munit_assert_uint64(timespan_as_secs(timespan_from_millis(999)), ==, 0);
    munit_assert_uint64(timespan_as_secs(timespan_from_millis(1001)), ==, 1);

    munit_assert_uint64(timespan_as_secs(timespan_from_micros(999999)), ==, 0);
    munit_assert_uint64(timespan_as_secs(timespan_from_micros(1000001)), ==, 1);

    munit_assert_uint64(timespan_as_secs(timespan_from_nanos(999999999)), ==, 0);
    munit_assert_uint64(timespan_as_secs(timespan_from_nanos(1000000001)), ==, 1);

    return MUNIT_OK;
}

MunitResult should_convert_to_millis(const MunitParameter params[], void* data)
{
    munit_assert_uint64(timespan_as_millis(timespan_from_secs(1)), ==, 1000);

    munit_assert_uint64(timespan_as_millis(timespan_from_millis(999)), ==, 999);
    munit_assert_uint64(timespan_as_millis(timespan_from_millis(1001)), ==, 1001);

    munit_assert_uint64(timespan_as_millis(timespan_from_micros(999)), ==, 0);
    munit_assert_uint64(timespan_as_millis(timespan_from_micros(1001)), ==, 1);

    munit_assert_uint64(timespan_as_millis(timespan_from_nanos(999999)), ==, 0);
    munit_assert_uint64(timespan_as_millis(timespan_from_nanos(1000001)), ==, 1);

    return MUNIT_OK;
}

MunitResult should_convert_to_micros(const MunitParameter params[], void* data)
{
    munit_assert_uint64(timespan_as_micros(timespan_from_secs(1)), ==, 1000000);

    munit_assert_uint64(timespan_as_micros(timespan_from_millis(999)), ==, 999000);
    munit_assert_uint64(timespan_as_micros(timespan_from_millis(1001)), ==, 1001000);

    munit_assert_uint64(timespan_as_micros(timespan_from_micros(999)), ==, 999);
    munit_assert_uint64(timespan_as_micros(timespan_from_micros(1001)), ==, 1001);

    munit_assert_uint64(timespan_as_micros(timespan_from_nanos(999)), ==, 0);
    munit_assert_uint64(timespan_as_micros(timespan_from_nanos(1001)), ==, 1);

    return MUNIT_OK;
}

MunitResult should_convert_to_nanos(const MunitParameter params[], void* data)
{
    munit_assert_uint64(timespan_as_nanos(timespan_from_secs(1)), ==, 1000000000);

    munit_assert_uint64(timespan_as_nanos(timespan_from_millis(999)), ==, 999000000);
    munit_assert_uint64(timespan_as_nanos(timespan_from_millis(1001)), ==, 1001000000);

    munit_assert_uint64(timespan_as_nanos(timespan_from_micros(999)), ==, 999000);
    munit_assert_uint64(timespan_as_nanos(timespan_from_micros(1001)), ==, 1001000);

    munit_assert_uint64(timespan_as_nanos(timespan_from_nanos(999)), ==, 999);
    munit_assert_uint64(timespan_as_nanos(timespan_from_nanos(1001)), ==, 1001);

    return MUNIT_OK;
}

MunitResult should_zero(const MunitParameter params[], void* data)
{
    timespan non_zero_ts = timespan_from_secs(1);
    timespan zero_ts;

    timespan_zero(&zero_ts);

    munit_assert_true(timespan_is_zero(non_zero_ts) == cebus_false);
    munit_assert_true(timespan_is_zero(zero_ts) == cebus_true);

    return MUNIT_OK;
}

CEBUS_DECLARE_TEST_SUITE(
    "timespan_tests",
    CEBUS_TEST(should_convert_to_secs),
    CEBUS_TEST(should_convert_to_millis),
    CEBUS_TEST(should_convert_to_micros),
    CEBUS_TEST(should_convert_to_nanos),
    CEBUS_TEST(should_zero)
);
