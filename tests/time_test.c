#include "suite.h"
#include "cebus/utils/time.h"

#include <stdio.h>

const struct test_data_ymd
{
    uint32_t year;
    uint32_t month;
    uint32_t day;

    cebus_bool is_valid;
} test_data_ymd_cases[] = {
    { 2022, 1, 10, cebus_true },
    { 2345, 10, 15, cebus_true },
    { 1582, 1, 1, cebus_true },

    { 1800, 0, 10, cebus_false },
    { 1800, 14, 10, cebus_false },
    { 2022, 3, 35, cebus_false },
    { 10000, 1, 1, cebus_false }
};

const struct test_data_ymd_hms
{
    uint32_t year;
    uint32_t month;
    uint32_t day;

    uint32_t hour;
    uint32_t minute;
    uint32_t second;

    cebus_bool is_valid;
} test_data_ymd_hms_cases[] = {
    { 2022, 1, 10, 10, 15, 32, cebus_true },
    { 2345, 10, 15, 20, 45, 12, cebus_true },
    { 1582, 1, 1, 8, 10, 32, cebus_true },
    { 2022, 1, 10, 0, 15, 32, cebus_true },

    { 2022, 1, 10, 26, 30, 15, cebus_false },
    { 2022, 1, 10, 13, 62, 15, cebus_false },
    { 2022, 1, 10, 15, 10, 70, cebus_false },
};

static const size_t test_data_ymd_size = sizeof test_data_ymd_cases / sizeof *test_data_ymd_cases;
static const size_t test_data_ymd_hms_size = sizeof test_data_ymd_hms_cases / sizeof *test_data_ymd_hms_cases;

MunitResult should_create_date_from_ymd(const MunitParameter params[], void* data)
{
    size_t i;
    for (i = 0; i < test_data_ymd_size; ++i)
    {
        const struct test_data_ymd* test = test_data_ymd_cases + i;
        cb_date_time dt = cb_date_time_from_ymd(test->year, test->month, test->day);

        munit_assert_true(cb_date_time_valid(dt) == test->is_valid);
    }

    return MUNIT_OK;
}

MunitResult should_get_year(const MunitParameter params[], void* data)
{
    size_t i;
    for (i = 0; i < test_data_ymd_size; ++i)
    {
        const struct test_data_ymd* test = test_data_ymd_cases + i;
        cb_date_time dt;

        if (test->is_valid == cebus_false)
            continue;

        dt = cb_date_time_from_ymd(test->year, test->month, test->day);
        munit_assert_uint32(cb_date_time_year(dt), ==, test->year);
    }

    return MUNIT_OK;
}

MunitResult should_get_month(const MunitParameter params[], void* data)
{
    size_t i;
    for (i = 0; i < test_data_ymd_size; ++i)
    {
        const struct test_data_ymd* test = test_data_ymd_cases + i;
        cb_date_time dt;

        if (test->is_valid == cebus_false)
            continue;

        dt = cb_date_time_from_ymd(test->year, test->month, test->day);
        munit_assert_uint32(cb_date_time_month(dt), ==, test->month);
    }

    return MUNIT_OK;
}

MunitResult should_get_day(const MunitParameter params[], void* data)
{
    size_t i;
    for (i = 0; i < test_data_ymd_size; ++i)
    {
        const struct test_data_ymd* test = test_data_ymd_cases + i;
        cb_date_time dt;

        if (test->is_valid == cebus_false)
            continue;

        dt = cb_date_time_from_ymd(test->year, test->month, test->day);
        munit_assert_uint32(cb_date_time_day(dt), ==, test->day);
    }

    return MUNIT_OK;
}

MunitResult should_create_date_from_ymd_hms(const MunitParameter params[], void* data)
{
    size_t i;
    for (i = 0; i < test_data_ymd_hms_size; ++i)
    {
        const struct test_data_ymd_hms* test = test_data_ymd_hms_cases + i;
        cb_date_time dt = cb_date_time_from_ymd_hms(test->year, test->month, test->day, test->hour, test->minute, test->second);
        munit_assert_true(cb_date_time_valid(dt) == test->is_valid);
    }

    return MUNIT_OK;
}

MunitResult should_get_hours(const MunitParameter params[], void* data)
{
    size_t i;
    for (i = 0; i < test_data_ymd_hms_size; ++i)
    {
        const struct test_data_ymd_hms* test = test_data_ymd_hms_cases + i;
        cb_date_time dt;

        if (test->is_valid == cebus_false)
            continue;

        dt = cb_date_time_from_ymd_hms(test->year, test->month, test->day, test->hour, test->minute, test->second);
        munit_assert_uint32(cb_date_time_hours(dt), ==, test->hour);
    }

    return MUNIT_OK;
}

MunitResult should_get_minutes(const MunitParameter params[], void* data)
{
    size_t i;
    for (i = 0; i < test_data_ymd_hms_size; ++i)
    {
        const struct test_data_ymd_hms* test = test_data_ymd_hms_cases + i;
        cb_date_time dt;

        if (test->is_valid == cebus_false)
            continue;

        dt = cb_date_time_from_ymd_hms(test->year, test->month, test->day, test->hour, test->minute, test->second);
        munit_assert_uint32(cb_date_time_minutes(dt), ==, test->minute);
    }

    return MUNIT_OK;
}

MunitResult should_get_seconds(const MunitParameter params[], void* data)
{
    size_t i;
    for (i = 0; i < test_data_ymd_hms_size; ++i)
    {
        const struct test_data_ymd_hms* test = test_data_ymd_hms_cases + i;
        cb_date_time dt;

        if (test->is_valid == cebus_false)
            continue;

        dt = cb_date_time_from_ymd_hms(test->year, test->month, test->day, test->hour, test->minute, test->second);
        munit_assert_uint32(cb_date_time_seconds(dt), ==, test->second);
    }

    return MUNIT_OK;
}

MunitResult should_get_utc_now(const MunitParameter params[], void* data)
{
    cb_date_time utc_now = cb_date_time_utc_now();
    char buf[100];
    memset(buf, 0, sizeof(buf));

    cb_date_time_print(utc_now, buf, sizeof(buf));

    munit_assert_string_equal(buf, "2022/1/12");
    return MUNIT_OK;
}

CEBUS_DECLARE_TEST_SUITE(
    time_tests,

    CEBUS_TEST(should_create_date_from_ymd),
    CEBUS_TEST(should_get_year),
    CEBUS_TEST(should_get_month),
    CEBUS_TEST(should_get_day),

    CEBUS_TEST(should_create_date_from_ymd_hms),
    CEBUS_TEST(should_get_hours),
    CEBUS_TEST(should_get_minutes),
    CEBUS_TEST(should_get_seconds),
    CEBUS_TEST(should_get_utc_now)
)
