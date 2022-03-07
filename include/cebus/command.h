#pragma once

#include "cebus/message_type_id.h"
#include "protobuf-c/protobuf-c.h" 

typedef struct cb_command
{
    void* data;

    size_t n_data;

    cb_message_type_id message_type_id;
} cb_command;

/// Create a `cb_command` from a `ProtobufCMessage`
cb_command cb_command_from_proto(const ProtobufCMessage* message, const char* proto_namespace);

#define CB_COMMAND(proto_msg, proto_namespace) cb_command_from_proto((const ProtobufCMessage *) &proto_msg, proto_namespace)
