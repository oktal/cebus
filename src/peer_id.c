#include "peer_id.h"


void peer_id_set(peer_id* peer, const char* value)
{
    strncpy(peer->value, value, PEER_ID_MAX);
}

const char* peer_id_get(const peer_id* peer)
{
    return peer->value;
}
