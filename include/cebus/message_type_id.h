#pragma once

#include "cebus/config.h"
#include "cebus/proto_types.h"

#include "message_type_id.pb-c.h"

typedef struct message_type_id
{
    char value[CEBUS_STR_MAX];
} message_type_id;

void message_type_id_set(message_type_id* type_id, const char* value);

MessageTypeId* message_type_id_proto_new(const message_type_id* type_id);
void message_type_id_proto_free(MessageTypeId* proto);
