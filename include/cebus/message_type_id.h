#pragma once

#include "cebus/config.h"
#include "cebus/cebus_bool.h"

#include "message_type_id.pb-c.h"

#define CB_MESSAGE_TYPE_ID_END_OF_STREAM "Abc.Zebus.Transport.EndOfStream"
#define CB_MESSAGE_TYPE_ID_END_OF_STREAM_ACK "Abc.Zebus.Transport.EndOfStreamAck"

typedef struct message_type_id
{
    char value[CEBUS_STR_MAX];
} message_type_id;

void message_type_id_set(message_type_id* type_id, const char* value);
cebus_bool message_type_id_equal(const message_type_id* type_id, const char* value);

MessageTypeId* message_type_id_proto_new(const message_type_id* type_id);
void message_type_id_proto_free(MessageTypeId* proto);
