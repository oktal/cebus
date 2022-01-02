#include "cebus/peer.h"

#include "cebus/alloc.h"
#include "cebus/peer_id.h"

#include <stdlib.h>
#include <string.h>

void cb_peer_set_endpoint(cb_peer* peer, const char* value)
{
    if (peer == NULL)
        return;

    strncpy(peer->endpoint, value, CEBUS_ENDPOINT_MAX);
}

Peer* cb_peer_proto_new(const cb_peer* peer)
{
    Peer* proto = cebus_alloc(sizeof *proto);
    peer__init(proto);
    proto->id = cb_peer_id_proto_new(&peer->peer_id);

    return proto;
}

void cb_peer_proto_free(Peer* peer)
{
    if (peer == NULL)
        return;

    free(peer->id);
    free(peer);
}
