#pragma once

#include "munit.h"

#define CEBUS_TEST_MAIN(suite)                             \
    int main(int argc, const char* argv[])                 \
    {                                                      \
        return munit_suite_main(&suite, NULL, argc, argv); \
    }

#define CEBUS_TEST(func)         \
    {                            \
        #func,                   \
        func,                    \
        NULL,                    \
        NULL,                    \
        MUNIT_TEST_OPTION_NONE,  \
        NULL                     \
    }

#define CEBUS_DECLARE_TEST_SUITE(name, ...)                      \
    MunitTest tests[] = {                                        \
        __VA_ARGS__,                                             \
        { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL } \
    };                                                           \
                                                                 \
    static const MunitSuite suite = {                            \
        #name,                                                   \
        tests,                                                   \
        NULL,                                                    \
        1,                                                       \
        MUNIT_SUITE_OPTION_NONE                                  \
    };                                                           \
    CEBUS_TEST_MAIN(suite)
