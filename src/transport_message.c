#include "cebus/transport_message.h"

#include "cebus/alloc.h"
#include "cebus/config.h"
#include "cebus/message_context.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void cb_set_originator_info(originator_info* originator, const cb_peer_id* peer_id, const char* sender_endpoint)
{
    originator_info_set_sender_id(originator, peer_id);
    originator_info_set_sender_endpoint(originator, sender_endpoint);
    originator_info_set_sender_machine(originator, get_machine_name());
    originator_info_set_sender_initiator_user(originator, get_initiator_user_name());
}

cb_transport_message* cb_to_transport_message(
        const cb_command* command,
        cb_time_uuid_gen* uuid_gen,
        const cb_peer_id* peer_id,
        const char* sender_endpoint,
        const char* environment)
{
    cb_transport_message* message = cb_new(cb_transport_message, 1);
    cb_message_id_next(&message->id, uuid_gen);
    cb_message_type_id_copy(&message->message_type_id, &command->message_type_id);
    strncpy(message->environment, environment, CEBUS_STR_MAX);
    message->data = command->data;
    message->n_data = command->n_data;
    cb_set_originator_info(&message->originator, peer_id, sender_endpoint);

    return message;
}

cb_transport_message* cb_transport_message_from_proto(cb_transport_message* message, const TransportMessage* proto)
{
    cb_message_id_from_proto(&message->id, proto->id);
    cb_message_type_id_from_proto(&message->message_type_id, proto->message_type_id);
    cb_originator_info_from_proto(&message->originator, proto->originator);
    strncpy(message->environment, proto->environment, CEBUS_STR_MAX);
    message->was_persisted = cebus_bool_from_int(proto->was_persisted);
    message->data = cb_alloc(proto->content_bytes.len);
    memcpy(message->data, proto->content_bytes.data, proto->content_bytes.len);
    message->n_data = proto->content_bytes.len;

    return message;
}

cb_transport_message* cb_transport_message_from_proto_new(const TransportMessage* proto)
{
    return cb_transport_message_from_proto(cb_new(cb_transport_message, 1), proto);
}

TransportMessage* cb_transport_message_proto_new(const cb_transport_message* message)
{
    TransportMessage* proto = cb_new(TransportMessage, 1);
    transport_message__init(proto);

    proto->id = cb_message_id_proto_new(&message->id);
    proto->message_type_id = cb_message_type_id_proto_new(&message->message_type_id);
    proto->content_bytes.data = cb_alloc(message->n_data);
    memcpy(proto->content_bytes.data, message->data, message->n_data);
    proto->content_bytes.len = message->n_data;
    proto->originator = originator_info_proto_new(&message->originator);
    proto->environment = cb_strdup(message->environment);
    proto->was_persisted = !!message->was_persisted;

    return proto;
}

void cb_transport_message_proto_free(TransportMessage* proto)
{
    cb_message_id_proto_free(proto->id);
    cb_message_type_id_proto_free(proto->message_type_id);
    free(proto->content_bytes.data);
    originator_info_proto_free(proto->originator);
    free(proto->environment);
    free(proto);
}
