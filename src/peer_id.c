#include "cebus/peer_id.h"

#include "cebus/alloc.h"
#include "cebus/config.h"

#include <stdlib.h>
#include <string.h>

void cb_peer_id_set(cb_peer_id* peer, const char* value)
{
    strncpy(peer->value, value, CEBUS_PEER_ID_MAX);
    peer->proto.value = peer->value;
}

const char* cb_peer_id_get(const cb_peer_id* peer)
{
    return peer->value;
}

PeerId* cb_peer_id_proto_new(const cb_peer_id* id)
{
    PeerId* proto = cb_new(PeerId, 1);
    peer_id__init(proto);

    proto->value = cb_strdup(id->value);
    return proto;
}

void cb_peer_id_proto_free(PeerId* proto)
{
    free(proto->value);
    free(proto);
}

void cb_peer_id_hash(cb_hasher* hasher, cb_hash_key_t peer_key)
{
    const cb_peer_id* peer_id = (cb_peer_id *)peer_key;
    cb_hasher_write_str(hasher, peer_id->value);
}

cebus_bool cb_peer_id_hash_eq(cb_hash_key_t lhs, cb_hash_key_t rhs)
{
    const cb_peer_id *p0 = (cb_peer_id *) lhs;
    const cb_peer_id *p1 = (cb_peer_id *) rhs;
    return cebus_bool_from_int(strncmp(p0->value, p1->value, CEBUS_PEER_ID_MAX) == 0);
}
