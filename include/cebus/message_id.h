#pragma once

#include "cebus/proto_types.h"
#include "message_id.pb-c.h"

#include <uuid/uuid.h>

typedef struct message_id
{
    uuid_t value;
} message_id;

void message_id_next(message_id* message_id);
MessageId* message_id_proto_new(const message_id* message_id);
void message_id_proto_free(MessageId* proto);
