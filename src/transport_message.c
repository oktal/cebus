#include "cebus/transport_message.h"

#include "cebus/alloc.h"
#include "cebus/config.h"
#include "cebus/message_context.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void transport_set_message_type_id_from_proto(transport_message* transport_message, const ProtobufCMessage* proto, const char* namespace)
{
    char full_name[CEBUS_STR_MAX];
    const ProtobufCMessageDescriptor* descriptor = proto->descriptor;

    snprintf(full_name, CEBUS_STR_MAX, "%s.%s", namespace, descriptor->name);
    message_type_id_set(&transport_message->message_type_id, full_name);
}

void* pack_message(const ProtobufCMessage* proto, size_t *size_out)
{
    const size_t packed_size = protobuf_c_message_get_packed_size(proto);
    if (size_out != NULL)
        *size_out = packed_size;

    uint8_t* buf = cebus_alloc(packed_size * sizeof(*buf));
    protobuf_c_message_pack(proto, buf);

    return buf;
}

static void set_originator_info(originator_info* originator, const peer_id* peer_id, const char* sender_endpoint)
{
    originator_info_set_sender_id(originator, peer_id);
    originator_info_set_sender_endpoint(originator, sender_endpoint);
    originator_info_set_sender_machine(originator, get_machine_name());
    originator_info_set_sender_initiator_user(originator, get_initiator_user_name());
}

transport_message* to_transport_message(const ProtobufCMessage* proto, const peer_id* peer_id, const char* sender_endpoint, const char* environment, const char* namespace)
{
    transport_message* message = cebus_alloc(sizeof* message);
    message_id_next(&message->id);
    transport_set_message_type_id_from_proto(message, proto, namespace);
    strncpy(message->environment, environment, CEBUS_STR_MAX);
    message->data = pack_message(proto, &message->n_data);
    set_originator_info(&message->originator, peer_id, sender_endpoint);

    return message;
}

TransportMessage* transport_message_proto_new(const transport_message* message)
{
    TransportMessage* proto = cebus_alloc(sizeof *proto);
    transport_message__init(proto);

    proto->id = message_id_proto_new(&message->id);
    proto->message_type_id = message_type_id_proto_new(&message->message_type_id);
    proto->content_bytes.data = cebus_alloc(message->n_data);
    memcpy(proto->content_bytes.data, message->data, message->n_data);
    proto->content_bytes.len = message->n_data;
    proto->originator = originator_info_proto_new(&message->originator);
    proto->environment = cebus_strdup(message->environment);
    proto->was_persisted = !!message->was_persisted;

    return proto;
}

void transport_message_proto_free(TransportMessage* proto)
{
    message_id_proto_free(proto->id);
    message_type_id_proto_free(proto->message_type_id);
    free(proto->content_bytes.data);
    originator_info_proto_free(proto->originator);
    free(proto->environment);
    free(proto);
}
