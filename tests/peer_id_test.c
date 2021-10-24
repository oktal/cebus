#include "munit.h"

#include "peer_id.h"

MunitResult should_set_peer_id_value(const MunitParameter params[], void* data)
{
    peer_id peerid;
    peer_id_set(&peerid, "Peer.Test.0");
    munit_assert_string_equal(peer_id_get(&peerid), "Peer.Test.0");
    return MUNIT_OK;
}

MunitTest tests[] = {
    {
        "should_set_peer_id_value",
        should_set_peer_id_value,
        NULL,
        NULL,
        MUNIT_TEST_OPTION_NONE,
        NULL
    },

    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite suite = {
    "peer_id_tests",
    tests,
    NULL,
    1,
    MUNIT_SUITE_OPTION_NONE
};

int main(int argc, const char* argv[])
{
    return munit_suite_main(&suite, NULL, argc, argv);
}
