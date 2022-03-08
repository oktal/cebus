#include "cebus/dispatch/message_proto_invoker.h"

#include "cebus/alloc.h"
#include "cebus/log.h"

typedef struct cb_message_proto_invoker_entry
{
    cb_message_type_id message_type_id;

    const ProtobufCMessageDescriptor* descriptor;

    cb_message_proto_handler handler;

    void* user;
} cb_message_proto_invoker_entry;

#define CB_MESSAGE_PROTO_INVOKER_INIT_CAPACITY 32

static void cb_message_proto_invoker_handler(const cb_transport_message* transport_message, void* user)
{
    cb_message_proto_invoker_entry* entry = (cb_message_proto_invoker_entry *) user;
    ProtobufCMessage* message = protobuf_c_message_unpack(entry->descriptor, NULL, transport_message->n_data, transport_message->data);
    if (message == NULL)
    {
        CB_LOG_DBG(CB_LOG_LEVEL_ERROR, "Failed to deserialized message of type %s from peer %s (%s)",
                transport_message->message_type_id.value, transport_message->originator.sender_id.value, transport_message->originator.sender_endpoint);
        return;
    }

    entry->handler(CB_MOVE(message), entry->user);
}

cb_message_proto_invoker* cb_message_proto_invoker_init(cb_message_proto_invoker* invoker, cb_bus* bus)
{
    cb_array_init_with_capacity(&invoker->invokers, CB_MESSAGE_PROTO_INVOKER_INIT_CAPACITY, sizeof(cb_message_proto_invoker_entry));
    invoker->bus = bus;
    return invoker;
}

void cb_message_proto_invoker_add(cb_message_proto_invoker* invoker, const ProtobufCMessage* message, const char* namespace, cb_message_proto_handler handler, void* user)
{
    cb_message_proto_invoker_entry* entry = (cb_message_proto_invoker_entry *) cb_array_push(&invoker->invokers);
    cb_message_type_id_from_proto_message(&entry->message_type_id, message, namespace);
    entry->descriptor = message->descriptor;
    entry->handler = handler;
    entry->user = user;

    cb_bus_handle_with(invoker->bus, &entry->message_type_id, cb_message_proto_invoker_handler, entry);
}

