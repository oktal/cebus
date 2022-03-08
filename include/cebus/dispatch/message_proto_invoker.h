#pragma once

#include "cebus/bus.h"
#include "cebus/collection/array.h"
#include "cebus/message_type_id.h"

#include "protobuf-c/protobuf-c.h"

typedef void (*cb_message_proto_handler)(ProtobufCMessage* message, void* user);

typedef struct cb_message_proto_invoker
{
    cb_bus* bus;

    cb_array invokers;
} cb_message_proto_invoker;

cb_message_proto_invoker* cb_message_proto_invoker_init(cb_message_proto_invoker* invoker, cb_bus* bus);
void cb_message_proto_invoker_add(cb_message_proto_invoker* invoker, const ProtobufCMessage* message, const char* namespace, cb_message_proto_handler handler, void* user);

