#pragma once

#include "message_id.pb-c.h"
#include "cebus/uuid.h"

typedef struct cb_message_id
{
    cb_uuid_t value;
} cb_message_id;

void cb_message_id_next(cb_message_id* message_id, cb_time_uuid_gen* gen);

MessageId* cb_message_id_proto_new(const cb_message_id* message_id);
void cb_message_id_proto_free(MessageId* proto);
