#include "cebus/command.h"

#include "cebus/alloc.h"
#include "message_serializer.h"

#include <stdio.h>

cb_command cb_command_from_proto(const ProtobufCMessage *message, const char *proto_namespace)
{
    cb_command command;
    char full_name[CEBUS_STR_MAX];
    const ProtobufCMessageDescriptor* descriptor = message->descriptor;

    snprintf(full_name, CEBUS_STR_MAX, "%s.%s", proto_namespace, descriptor->name);
    cb_message_type_id_set(&command.message_type_id, full_name);
    command.data = cb_pack_message(message , &command.n_data);
    return command;
}
