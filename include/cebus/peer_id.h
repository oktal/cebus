#pragma once

#include "cebus/config.h"

#include "peer_id.pb-c.h"

typedef struct peer_id
{
    char value[CEBUS_PEER_ID_MAX];
    PeerId proto;
} peer_id;

void peer_id_set(peer_id* peer, const char* value);
const char* peer_id_get(const peer_id* peer);

PeerId* peer_id_proto_new(const peer_id* id);
void peer_id_proto_free(PeerId* proto);
