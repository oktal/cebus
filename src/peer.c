#include "cebus/peer.h"

#include "cebus/alloc.h"
#include "cebus/peer_id.h"

#include <stdlib.h>
#include <string.h>

void peer_set_endpoint(peer* peer, const char* value)
{
    if (peer == NULL)
        return;

    strncpy(peer->endpoint, value, CEBUS_ENDPOINT_MAX);
}

Peer* peer_proto_new(const peer* peer)
{
    Peer* proto = cebus_alloc(sizeof *proto);
    peer__init(proto);
    proto->id = peer_id_proto_new(&peer->id);

    return proto;
}

void peer_proto_free(Peer* peer)
{
    if (peer == NULL)
        return;

    free(peer->id);
    free(peer);
}
