#include "suite.h"
#include "cebus/peer_id.h"

MunitResult should_set_peer_id_value(const MunitParameter params[], void* data)
{
    cb_peer_id peerid;
    cb_peer_id_set(&peerid, "Peer.Test.0");
    munit_assert_string_equal(cb_peer_id_get(&peerid), "Peer.Test.0");
    return MUNIT_OK;
}

CEBUS_DECLARE_TEST_SUITE(
    "peer_id_tests",
    CEBUS_TEST(should_set_peer_id_value)
);
