#pragma once

#include "cebus/collection/hash_map.h"
#include "cebus/config.h"
#include "cebus/cebus_bool.h"

#include "message_type_id.pb-c.h"

#define CB_MESSAGE_TYPE_ID_END_OF_STREAM "Abc.Zebus.Transport.EndOfStream"
#define CB_MESSAGE_TYPE_ID_END_OF_STREAM_ACK "Abc.Zebus.Transport.EndOfStreamAck"
#define CB_MESSAGE_TYPE_ID_MESSAGE_EXECUTION_COMPLETED "Abc.Zebus.Core.MessageExecutionCompleted"

typedef struct cb_message_type_id
{
    char value[CEBUS_STR_MAX];
} cb_message_type_id;

void cb_message_type_id_set(cb_message_type_id* type_id, const char* fmt, ...);

cb_message_type_id* cb_message_type_id_from_proto_message(cb_message_type_id* type_id, const ProtobufCMessage* message, const char* namespace);
cb_message_type_id* cb_message_type_id_copy(cb_message_type_id* dst, const cb_message_type_id* src);

cb_message_type_id* cb_message_type_id_clone(const cb_message_type_id* src);

cebus_bool cb_message_type_id_eq_str(const cb_message_type_id* type_id, const char* value);
cebus_bool cb_message_type_id_eq(const cb_message_type_id* lhs, const cb_message_type_id* rhs);

/// Initialize a new `cb_message_type_id` from a `MessageTypeId` protobuf message
cb_message_type_id* cb_message_type_id_from_proto(cb_message_type_id* type_id, const MessageTypeId* proto);

MessageTypeId* cb_message_type_id_proto_new(const cb_message_type_id* type_id);
void cb_message_type_id_proto_free(MessageTypeId* proto);

void cb_message_type_id_hash(cb_hasher* hasher, cb_hash_key_t message_type_id_key);
cebus_bool cb_message_type_id_hash_eq(cb_hash_key_t lhs, cb_hash_key_t rhs);
