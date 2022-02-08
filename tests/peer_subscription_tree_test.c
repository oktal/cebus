#include "suite.h"

#include "cebus/alloc.h"
#include "cebus/binding_key.h"
#include "cebus/peer.h"
#include "cebus/peer_subscription_tree.h"

static void check_peer(cb_peer_list_iterator iter, const char* peer_id)
{
    munit_assert(cb_peer_list_iter_has_next(iter));
    {
        const cb_peer* peer = cb_peer_list_iter_peer(iter);
        munit_assert_not_null(peer);
        munit_assert_string_equal(peer->peer_id.value, peer_id);
    }
}

MunitResult empty_routing_key_should_match_all_peers(const MunitParameter params[], void* data)
{
    cb_peer peer_1, peer_2;
    cb_binding_key empty_key;

    const char* keys[] = {
        "*",
        "#",
        "zig",
        "zig.*",
        "zig.#",
        "zig.zag.*",
        "zig.zag.zog",
        "zig.*.zog"
    };
    const size_t key_count = sizeof keys / sizeof *keys;
    size_t i;

    cb_binding_key_init(&empty_key);

    cb_peer_id_set(&peer_1.peer_id, "My.Peer.0");
    cb_peer_set_endpoint(&peer_1, "tcp://hostname:9080");

    cb_peer_id_set(&peer_2.peer_id, "My.Peer.1");
    cb_peer_set_endpoint(&peer_2, "tcp://hostname:9090");

    for (i = 0; i < key_count; ++i)
    {
        cb_peer_subscription_tree tree;

        cb_binding_key key_1 = cb_binding_key_from_str(keys[i]);
        cb_binding_key key_2 = cb_binding_key_from_str("other.key.0");

        cb_peer_list peers;
        cb_peer_list_iterator it;

        printf("/%s\n", keys[i]);

        cb_peer_subscription_tree_init(&tree);
        cb_peer_subscription_tree_add(&tree, &peer_1, key_1);
        cb_peer_subscription_tree_add(&tree, &peer_2, key_2);

        peers = cb_peer_subscription_tree_get_peers(&tree, empty_key);
        it = cb_peer_list_iter(&peers);

        check_peer(it, "My.Peer.0");
        cb_peer_list_iter_move_next(&it);

        check_peer(it, "My.Peer.1");
        cb_peer_list_iter_move_next(&it);

        munit_assert(cb_peer_list_iter_has_next(it) == cebus_false);

        cb_binding_key_free(&key_1);
        cb_binding_key_free(&key_2);

        cb_peer_subscription_tree_free(&tree);
    }

    return MUNIT_OK;
}

MunitResult should_match_star(const MunitParameter params[], void* data)
{
    cb_peer_subscription_tree tree;
    cb_peer peer;
    cb_binding_key binding_key = cb_binding_key_from_str("*");

    const char* keys[] = {
        "zig",
        "*",
        "#",
    };
    const size_t key_count = sizeof keys / sizeof *keys;
    size_t i;

    cb_peer_subscription_tree_init(&tree);
    cb_peer_id_set(&peer.peer_id, "My.Peer.0");
    cb_peer_set_endpoint(&peer, "tcp://hostname:9080");

    cb_peer_subscription_tree_add(&tree, &peer, binding_key);

    for (i = 0; i < key_count; ++i)
    {
        cb_binding_key key = cb_binding_key_from_str(keys[i]);
        cb_peer_list peers;
        cb_peer_list_iterator it;

        printf("/%s\n", keys[i]);

        peers = cb_peer_subscription_tree_get_peers(&tree, key);
        it = cb_peer_list_iter(&peers);

        check_peer(it, "My.Peer.0");
        cb_binding_key_free(&key);
    }

    cb_binding_key_free(&binding_key);
    cb_peer_subscription_tree_free(&tree);

    return MUNIT_OK;
}

MunitResult should_match_simple_key(const MunitParameter params[], void* data)
{
    cb_peer_subscription_tree tree;
    cb_peer peer_1;
    cb_peer peer_2;
    cb_peer_list peers;
    cb_peer_list_iterator it;
    const cb_peer* peer;

    cb_binding_key binding_key_1 = cb_binding_key_from_str("zig");
    cb_binding_key binding_key_2 = cb_binding_key_from_str("zag");

    cb_peer_subscription_tree_init(&tree);

    cb_peer_id_set(&peer_1.peer_id, "My.Peer.0");
    cb_peer_set_endpoint(&peer_1, "tcp://hostname:9080");

    cb_peer_id_set(&peer_2.peer_id, "My.Peer.1");
    cb_peer_set_endpoint(&peer_2, "tcp://hostname:9090");

    cb_peer_subscription_tree_add(&tree, &peer_1, binding_key_1);
    cb_peer_subscription_tree_add(&tree, &peer_2, binding_key_2);

    cb_test_case("Assert that `My.Peer.0` and only `My.Peer.0` matches `binding_key_1`")
    {
        peers = cb_peer_subscription_tree_get_peers(&tree, binding_key_1);
        it = cb_peer_list_iter(&peers);

        check_peer(it, "My.Peer.0");
        cb_peer_list_iter_move_next(&it);

        munit_assert(cb_peer_list_iter_has_next(it) == cebus_false);
    }

    cb_test_case("Assert that `My.Peer.1` and only `My.Peer.1` matches `binding_key_1`")
    {
        peers = cb_peer_subscription_tree_get_peers(&tree, binding_key_2);
        it = cb_peer_list_iter(&peers);

        check_peer(it, "My.Peer.1");
        cb_peer_list_iter_move_next(&it);

        munit_assert(cb_peer_list_iter_has_next(it) == cebus_false);
    }

    cb_binding_key_free(&binding_key_1);
    cb_binding_key_free(&binding_key_2);
    cb_peer_subscription_tree_free(&tree);

    return MUNIT_OK;
}

MunitResult should_match_key_with_wildcard(const MunitParameter params[], void* data)
{
    cb_peer_subscription_tree tree;
    cb_peer peer_1;
    cb_peer peer_2;
    cb_peer_list peers;
    cb_peer_list_iterator it;
    const cb_peer* peer;

    cb_binding_key binding_key_1 = cb_binding_key_from_str("zig.*.123.*");
    cb_binding_key binding_key_2 = cb_binding_key_from_str("zig.zag.*.zug");

    cb_peer_subscription_tree_init(&tree);

    cb_peer_id_set(&peer_1.peer_id, "My.Peer.0");
    cb_peer_set_endpoint(&peer_1, "tcp://hostname:9080");

    cb_peer_id_set(&peer_2.peer_id, "My.Peer.1");
    cb_peer_set_endpoint(&peer_2, "tcp://hostname:9090");

    cb_peer_subscription_tree_add(&tree, &peer_1, binding_key_1);
    cb_peer_subscription_tree_add(&tree, &peer_2, binding_key_2);

    cb_test_case("Assert that we can match `binding_key_1` and only `binding_key_1`")
    {
        const char* keys[] = {
            "zig.zog.123.giz",
            "zig.zug.123.gaz",
        };
        const size_t key_count = sizeof keys / sizeof *keys;
        size_t i;

        for (i = 0; i < key_count; ++i)
        {
            cb_binding_key key = cb_binding_key_from_str(keys[i]);
            printf("/%s\n", keys[i]);

            peers = cb_peer_subscription_tree_get_peers(&tree, key);
            it = cb_peer_list_iter(&peers);

            check_peer(it, "My.Peer.0");
            cb_peer_list_iter_move_next(&it);

            munit_assert(cb_peer_list_iter_has_next(it) == cebus_false);

            cb_binding_key_free(&key);
        }
    }

    cb_test_case("Assert that we can match `binding_key_2` and only `binding_key_2`")
    {
        const char* keys[] = {
            "zig.zag.PING.zug",
            "zig.zag.PONG.zug",
        };

        const size_t key_count = sizeof keys / sizeof *keys;
        size_t i;

        for (i = 0; i < key_count; ++i)
        {
            cb_binding_key key = cb_binding_key_from_str(keys[i]);
            printf("/%s\n", keys[i]);

            peers = cb_peer_subscription_tree_get_peers(&tree, key);
            it = cb_peer_list_iter(&peers);

            check_peer(it, "My.Peer.1");
            cb_peer_list_iter_move_next(&it);

            munit_assert(cb_peer_list_iter_has_next(it) == cebus_false);

            cb_binding_key_free(&key);
        }
    }

    cb_test_case("Assert that we can match `binding_key_1` and `binding_key_2`")
    {
        const char* keys[] = {
            "zig.zag.123.zug"
        };

        const size_t key_count = sizeof keys / sizeof *keys;
        size_t i;

        for (i = 0; i < key_count; ++i)
        {
            cb_binding_key key = cb_binding_key_from_str(keys[i]);
            printf("/%s\n", keys[i]);

            peers = cb_peer_subscription_tree_get_peers(&tree, key);
            it = cb_peer_list_iter(&peers);

            check_peer(it, "My.Peer.0");
            cb_peer_list_iter_move_next(&it);

            check_peer(it, "My.Peer.1");
            cb_peer_list_iter_move_next(&it);

            munit_assert(cb_peer_list_iter_has_next(it) == cebus_false);

            cb_binding_key_free(&key);
        }
    }

    cb_binding_key_free(&binding_key_1);
    cb_binding_key_free(&binding_key_2);
    cb_peer_subscription_tree_free(&tree);

    return MUNIT_OK;
}

MunitResult should_match_key_with_all_levels_of_wildcards(const MunitParameter params[], void* data)
{
    const char* keys[] = {
        "zig.*.*",
        "zig.zag.*",
        "zig.zag.zog",
        "zig.*.zog",
        "*.zag.zog",
        "*.*.zog"
    };
    const size_t key_count = sizeof keys / sizeof *keys;
    size_t i;

    cb_binding_key test_key = cb_binding_key_from_str("zig.zag.zog");

    for (i = 0; i < key_count; ++i)
    {
        cb_binding_key key_1 = cb_binding_key_from_str(keys[i]);
        cb_binding_key key_2 = cb_binding_key_from_str("other.key.1");
        cb_peer peer_1, peer_2;
        cb_peer_subscription_tree tree;
        cb_peer_list list;
        cb_peer_list_iterator it;

        printf("/%s\n", keys[i]);

        cb_peer_id_set(&peer_1.peer_id, "My.Peer.0");
        cb_peer_set_endpoint(&peer_1, "tcp://hostname:9080");

        cb_peer_id_set(&peer_2.peer_id, "My.Peer.1");
        cb_peer_set_endpoint(&peer_2, "tcp://hostname:9070");

        cb_peer_subscription_tree_init(&tree);
        cb_peer_subscription_tree_add(&tree, &peer_1, key_1);
        cb_peer_subscription_tree_add(&tree, &peer_2, key_2);

        list = cb_peer_subscription_tree_get_peers(&tree, test_key);
        it = cb_peer_list_iter(&list);

        check_peer(it, "My.Peer.0");

        cb_peer_list_iter_move_next(&it);
        munit_assert(cb_peer_list_iter_has_next(it) == cebus_false);

        cb_binding_key_free(&key_1);
        cb_binding_key_free(&key_2);
        cb_peer_subscription_tree_free(&tree);
    }

    cb_binding_key_free(&test_key);

    return MUNIT_OK;
}

MunitResult should_match_key_with_all_levels_of_dashes(const MunitParameter params[], void* data)
{
    const char* keys[] = {
        "zig.#",
        "zig.zag.#",
    };
    const size_t key_count = sizeof keys / sizeof *keys;
    size_t i;

    cb_binding_key test_key = cb_binding_key_from_str("zig.zag.zog");

    for (i = 0; i < key_count; ++i)
    {
        cb_binding_key key_1 = cb_binding_key_from_str(keys[i]);
        cb_binding_key key_2 = cb_binding_key_from_str("other.key.1");
        cb_peer peer_1, peer_2;
        cb_peer_subscription_tree tree;
        cb_peer_list list;
        cb_peer_list_iterator it;

        printf("/%s\n", keys[i]);

        cb_peer_id_set(&peer_1.peer_id, "My.Peer.0");
        cb_peer_set_endpoint(&peer_1, "tcp://hostname:9080");

        cb_peer_id_set(&peer_2.peer_id, "My.Peer.1");
        cb_peer_set_endpoint(&peer_2, "tcp://hostname:9070");

        cb_peer_subscription_tree_init(&tree);
        cb_peer_subscription_tree_add(&tree, &peer_1, key_1);
        cb_peer_subscription_tree_add(&tree, &peer_2, key_2);

        list = cb_peer_subscription_tree_get_peers(&tree, test_key);
        it = cb_peer_list_iter(&list);

        check_peer(it, "My.Peer.0");

        cb_peer_list_iter_move_next(&it);
        munit_assert(cb_peer_list_iter_has_next(it) == cebus_false);

        cb_binding_key_free(&key_1);
        cb_binding_key_free(&key_2);
        cb_peer_subscription_tree_free(&tree);
    }

    cb_binding_key_free(&test_key);

    return MUNIT_OK;
}

MunitResult should_add_large_depth_of_keys(const MunitParameter params[], void* data)
{
    const size_t key_count = CB_SUBSCRIPTION_TREE_BUCKET_MAX * 10;
    size_t i;
    cb_binding_key* keys = cb_new(cb_binding_key, key_count);
    cb_peer_subscription_tree tree;
    cb_peer_subscription_tree_init(&tree);

    cb_peer peer;
    cb_peer_set_endpoint(&peer, "tcp://hostname:9098");
    cb_peer_id_set(&peer.peer_id, "My.Peer.0");

    for (i = 0; i < key_count; ++i)
    {
        keys[i] = cb_binding_key_from_str("zig.zag_%zu.*", i);
        cb_peer_subscription_tree_add(&tree, &peer, keys[i]);
    }

    for (i = 0; i < key_count; ++i)
    {
        cb_peer_list list = cb_peer_subscription_tree_get_peers(&tree, keys[i]);
        cb_peer_list_iterator it = cb_peer_list_iter(&list);

        check_peer(it, "My.Peer.0");
    }

    for (i = 0; i < key_count; ++i)
    {
        cb_binding_key_free(&keys[i]);
    }

    cb_peer_subscription_tree_free(&tree);
    free(keys);
    return MUNIT_OK;
}

MunitResult should_add_and_remove_large_depth_of_keys(const MunitParameter params[], void* data)
{
    const size_t key_count = CB_SUBSCRIPTION_TREE_BUCKET_MAX * 10;
    size_t i;
    cb_binding_key* keys = cb_new(cb_binding_key, key_count);
    cb_peer_subscription_tree tree;
    cb_peer_subscription_tree_init(&tree);

    cb_peer peer;
    cb_peer_set_endpoint(&peer, "tcp://hostname:9098");
    cb_peer_id_set(&peer.peer_id, "My.Peer.0");

    for (i = 0; i < key_count; ++i)
    {
        keys[i] = cb_binding_key_from_str("zig.zag_%zu.*", i);
        cb_peer_subscription_tree_add(&tree, &peer, keys[i]);
    }

    for (i = 0; i < key_count; ++i)
    {
        cb_peer_subscription_tree_remove(&tree, &peer, keys[i]);
    }

    for (i = 0; i < key_count; ++i)
    {
        cb_peer_list list = cb_peer_subscription_tree_get_peers(&tree, keys[i]);
        cb_peer_list_iterator it = cb_peer_list_iter(&list);

        munit_assert(cb_peer_list_iter_has_next(it) == cebus_false);
    }

    for (i = 0; i < key_count; ++i)
    {
        cb_binding_key_free(&keys[i]);
    }

    cb_peer_subscription_tree_free(&tree);
    free(keys);
    return MUNIT_OK;
}

CEBUS_DECLARE_TEST_SUITE(
    peer_subscription_tree,

    CEBUS_TEST(empty_routing_key_should_match_all_peers),
    CEBUS_TEST(should_match_star),
    CEBUS_TEST(should_match_simple_key),
    CEBUS_TEST(should_match_key_with_wildcard),
    CEBUS_TEST(should_match_key_with_all_levels_of_wildcards),
    CEBUS_TEST(should_match_key_with_all_levels_of_dashes),
    CEBUS_TEST(should_add_large_depth_of_keys),
    CEBUS_TEST(should_add_and_remove_large_depth_of_keys)
)
