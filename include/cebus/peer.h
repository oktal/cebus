#pragma once

#include "cebus/cebus_bool.h"
#include "cebus/config.h"
#include "cebus/peer_id.h"

#include "peer.pb-c.h"

typedef struct cb_peer
{
    cb_peer_id peer_id;
    char endpoint[CEBUS_ENDPOINT_MAX];

    cebus_bool is_up;
    cebus_bool is_responding;
} cb_peer;

void cb_peer_set_endpoint(cb_peer* peer, const char* value);

Peer* cb_peer_proto_new(const cb_peer* peer);
void cb_peer_proto_free(Peer* peer);
