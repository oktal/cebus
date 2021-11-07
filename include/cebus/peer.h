#pragma once

#include "cebus/cebus_bool.h"
#include "cebus/config.h"
#include "cebus/peer_id.h"

#include "peer.pb-c.h"

typedef struct peer
{
    peer_id id;
    char endpoint[CEBUS_ENDPOINT_MAX];

    cebus_bool is_up;
    cebus_bool is_responding;
} peer;

void peer_set_endpoint(peer* peer, const char* value);

Peer* peer_proto_new(const peer* peer);
void peer_proto_free(Peer* peer);
