#include "cebus/dispatch/message_proto_invoker.h"

#include "cebus/alloc.h"
#include "cebus/log.h"


#define CB_MESSAGE_PROTO_INVOKER_INIT_CAPACITY 32

static void cb_message_proto_invoker_handler(const cb_transport_message* transport_message, void* user)
{
    cb_message_proto_invoker_entry* entry = (cb_message_proto_invoker_entry *) user;
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

