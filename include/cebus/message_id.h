#pragma once

#include "cebus/collection/hash_map.h"
#include "cebus/uuid.h"

#include "message_id.pb-c.h"

typedef struct cb_message_id
{
    cb_uuid_t value;
} cb_message_id;

cb_message_id* cb_message_id_from_proto(cb_message_id* message_id, const MessageId* proto);
void cb_message_id_next(cb_message_id* message_id, cb_time_uuid_gen* gen);

MessageId* cb_message_id_proto_new(const cb_message_id* message_id);
void cb_message_id_proto_free(MessageId* proto);

void cb_message_id_hash(cb_hasher* hasher, cb_hash_key_t message_id_key);
cebus_bool cb_message_id_hash_eq(cb_hash_key_t lhs, cb_hash_key_t rhs);
