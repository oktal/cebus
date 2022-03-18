#pragma once

#include "cebus/bus.h"
#include "cebus/collection/hash_map.h"
#include "cebus/config.h"

#include "protobuf-cebus/protobuf-cebus.h"

typedef void (*cb_proto_message_handler)(ProtobufCMessage* message, void* user);

typedef struct cb_proto_message_dispatcher
{
    struct cb_proto_message_dispatcher_impl* impl;
} cb_proto_message_dispatcher;

cb_proto_message_dispatcher* cb_proto_message_dispatcher_init(cb_proto_message_dispatcher* dispatcher, cb_bus* bus);

void cb_proto_message_dispatcher_register(cb_proto_message_dispatcher* dispatcher, const ProtobufCebusMessage* message, cb_proto_message_handler handler, void* user);
void cb_proto_message_dispatcher_register_queue(cb_proto_message_dispatcher* dispatcher, const ProtobufCebusMessage* message, const char* dispatch_queue, cb_proto_message_handler handler, void* user);

void cb_proto_message_dispatcher_free(cb_proto_message_dispatcher* dispatcher);
