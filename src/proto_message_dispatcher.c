#include "cebus/dispatch/proto_message_dispatcher.h"
#include "cebus/alloc.h"
#include "cebus/collection/array.h"
#include "cebus/log.h"
#include "cebus/transport_message.h"

typedef struct cb_proto_message_dispatcher_entry
{
    const char* dispatch_queue;

    const ProtobufCebusMessageDescriptor* descriptor;

    cb_proto_message_handler handler;

    void* user;
} cb_proto_message_dispatcher_entry;

typedef struct cb_proto_message_dispatcher_impl
{
    cb_array invokers;

    cb_bus* bus;
} cb_proto_message_dispatcher_impl;


static cb_proto_message_dispatcher_impl* cb_proto_message_dispatcher_impl_init(cb_proto_message_dispatcher_impl* impl, cb_bus* bus)
{
    cb_array_init_with_capacity(&impl->invokers, 32, sizeof(cb_proto_message_dispatcher_entry));
    impl->bus = bus;
    return impl;
}

static void cb_proto_message_dispatcher_impl_free(cb_proto_message_dispatcher_impl* impl)
{
    cb_array_free(&impl->invokers, NULL, NULL);
}

static void cb_proto_message_dispatcher_invoke(const cb_transport_message* transport_message, void* user)
{
    cb_proto_message_dispatcher_entry* entry = (cb_proto_message_dispatcher_entry *) user;
    ProtobufCMessage* message = protobuf_c_message_unpack(entry->descriptor->descriptor, NULL, transport_message->n_data, transport_message->data);
    if (message == NULL)
    {
        CB_LOG_DBG(CB_LOG_LEVEL_WARN, "Failed to deserialize message type %s from Peer %s (%s)",
                transport_message->message_type_id.value, transport_message->originator.sender_id, transport_message->originator.sender_endpoint);
        return;
    }

    entry->handler(message, entry->user);
}

static void cb_proto_message_dispatcher_impl_register(cb_proto_message_dispatcher_impl* impl, const ProtobufCebusMessage* message, cb_proto_message_handler handler, void* user)
{
    cb_proto_message_dispatcher_entry* entry = (cb_proto_message_dispatcher_entry *) cb_array_push(&impl->invokers);
    entry->dispatch_queue = NULL;
    entry->descriptor = message->descriptor;
    entry->handler = handler;
    entry->user = user;

    {
        cb_message_handler_invoker invoker;
        cb_message_type_id message_type_id;

        cb_message_handler_invoker_init(&invoker, NULL, cb_proto_message_dispatcher_invoke, entry);
        cb_message_type_id_from_proto_message(&message_type_id, message);

        cb_bus_register_invoker(impl->bus, &message_type_id, &invoker);
    }
}

cb_proto_message_dispatcher* cb_proto_message_dispatcher_init(cb_proto_message_dispatcher* dispatcher, cb_bus* bus)
{
    dispatcher->impl = cb_proto_message_dispatcher_impl_init(cb_new(cb_proto_message_dispatcher_impl, 1), bus);
    return dispatcher;
}

void cb_proto_message_dispatcher_register(cb_proto_message_dispatcher* dispatcher, const ProtobufCebusMessage* message, cb_proto_message_handler handler, void* user)
{
    cb_proto_message_dispatcher_impl_register(dispatcher->impl, message, handler, user);
}

void cb_proto_message_dispatcher_free(cb_proto_message_dispatcher* dispatcher)
{
    cb_proto_message_dispatcher_impl_free(dispatcher->impl);
    free(dispatcher->impl);
}
