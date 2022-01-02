#pragma once

#include "cebus/collection/hash_map.h"
#include "cebus/config.h"

#include "peer_id.pb-c.h"

typedef struct cb_peer_id
{
    char value[CEBUS_PEER_ID_MAX];
    PeerId proto;
} cb_peer_id;

void cb_peer_id_set(cb_peer_id* peer, const char* value);
const char* cb_peer_id_get(const cb_peer_id* peer);

PeerId* cb_peer_id_proto_new(const cb_peer_id* id);
void cb_peer_id_proto_free(PeerId* proto);

void cb_peer_id_hash(cb_hasher* hasher, cb_hash_key_t peer_key);
cebus_bool cb_peer_id_hash_eq(cb_hash_key_t lhs, cb_hash_key_t rhs);
