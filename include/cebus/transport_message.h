#pragma once

#include "cebus/config.h"
#include "cebus/command.h"
#include "cebus/cebus_bool.h"

#include "cebus/message_id.h"
#include "cebus/message_type_id.h"
#include "cebus/originator_info.h"
#include "cebus/peer_id.h"

#include "transport_message.pb-c.h"

typedef struct cb_transport_message
{
    cb_message_id id;
    cb_message_type_id message_type_id;
    originator_info originator;

    char environment[CEBUS_STR_MAX];
    cebus_bool was_persisted;

    void *data;
    size_t n_data;
} cb_transport_message;

cb_transport_message* cb_to_transport_message(
        const cb_command* message,
        cb_time_uuid_gen* uuid_gen,
        const cb_peer_id* peer_id,
        const char* sender_endpoint,
        const char* environment);

TransportMessage* cb_transport_message_proto_new(const cb_transport_message* message);
void cb_transport_message_proto_free(TransportMessage* message);
