#pragma once

#include "cebus/config.h"
#include "cebus/peer_id.h"

#include "originator_info.pb-c.h"

typedef struct originator_info
{
    peer_id sender_id;
    char sender_endpoint[CEBUS_STR_MAX];
    char sender_machine[CEBUS_STR_MAX];
    char initiator_user_name[CEBUS_STR_MAX];
} originator_info;

void originator_info_set_sender_id(originator_info* originator, const peer_id* sender_id);
void originator_info_set_sender_endpoint(originator_info *info, const char* endpoint);
void originator_info_set_sender_machine(originator_info *info, const char* machine);
void originator_info_set_sender_initiator_user(originator_info *info, const char* user);

OriginatorInfo* originator_info_proto_new(const originator_info* info);
void originator_info_proto_free(OriginatorInfo* proto);
