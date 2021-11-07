#include "cebus/peer_id.h"

#include "cebus/alloc.h"
#include "cebus/config.h"

#include <stdlib.h>
#include <string.h>

void peer_id_set(peer_id* peer, const char* value)
{
    strncpy(peer->value, value, CEBUS_PEER_ID_MAX);
    peer->proto.value = peer->value;
}

const char* peer_id_get(const peer_id* peer)
{
    return peer->value;
}

PeerId* peer_id_proto_new(const peer_id* id)
{
    PeerId* proto = cebus_alloc(sizeof *proto);
    peer_id__init(proto);

    proto->value = cebus_strdup(id->value);
    return proto;
}

void peer_id_proto_free(PeerId* proto)
{
    free(proto->value);
    free(proto);
}
