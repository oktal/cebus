#include "cebus/originator_info.h"

#include "cebus/alloc.h"
#include "cebus/peer_id.h"

#include <stdlib.h>
#include <string.h>

void originator_info_set_sender_id(originator_info* originator, const cb_peer_id* sender_id)
{
    cb_peer_id_set(&originator->sender_id, sender_id->value);
}

void originator_info_set_sender_endpoint(originator_info* originator, const char* endpoint)
{
    strncpy(originator->sender_endpoint, endpoint, CEBUS_STR_MAX);
}

void originator_info_set_sender_machine(originator_info* originator, const char* machine)
{
    strncpy(originator->sender_machine, machine, CEBUS_STR_MAX);
}

void originator_info_set_sender_initiator_user(originator_info* originator, const char* user)
{
    strncpy(originator->initiator_user_name, user, CEBUS_STR_MAX);
}

originator_info* cb_originator_info_from_proto(originator_info* info, const OriginatorInfo* proto)
{
    if (proto->sender_id != NULL)
        cb_peer_id_set(&info->sender_id, proto->sender_id->value);
    if (proto->sender_endpoint != NULL)
        originator_info_set_sender_endpoint(info, proto->sender_endpoint);
    if (proto->sender_machine != NULL)
        originator_info_set_sender_machine(info, proto->sender_machine);
    if (proto->initiator_user_name != NULL)
        originator_info_set_sender_initiator_user(info, proto->initiator_user_name);

    return info;
}

OriginatorInfo* originator_info_proto_new(const originator_info* info)
{
    OriginatorInfo* proto = cb_new(OriginatorInfo, 1);
    originator_info__init(proto);

    proto->sender_id = cb_peer_id_proto_new(&info->sender_id);
    proto->sender_endpoint = cb_strdup(info->sender_endpoint);
    proto->sender_machine = cb_strdup(info->sender_machine);
    proto->initiator_user_name = cb_strdup(info->initiator_user_name);

    return proto;;
}

void originator_info_proto_free(OriginatorInfo* proto)
{
    free(proto->initiator_user_name);
    free(proto->sender_machine);
    free(proto->sender_endpoint);
    cb_peer_id_proto_free(proto->sender_id);
}
