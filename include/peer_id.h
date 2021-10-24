#pragma once

#include <string.h>

#if !defined(PEER_ID_MAX)
  #define PEER_ID_MAX 255
#endif

typedef struct peer_id
{
    char value[PEER_ID_MAX];
} peer_id;

void peer_id_set(peer_id* peer, const char* value);
const char* peer_id_get(const peer_id* peer);
