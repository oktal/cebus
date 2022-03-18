#include "cebus/command.h"
#include "cebus/alloc.h"

#include "message_serializer.h"

#include "protobuf-c/protobuf-c.h"

#include <string.h>

void cb_command_init(cb_command* command)
{
    command->data = NULL;
    command->n_data = 0;
}

cb_command* cb_command_copy(cb_command* dst, const cb_command* src)
{
    dst->data = cb_alloc(src->n_data);
    memcpy(dst->data, src->data, src->n_data);
    dst->n_data = src->n_data;

    cb_message_type_id_copy(&dst->message_type_id, &src->message_type_id);
    return dst;
}

cb_command* cb_command_move(cb_command* dst, cb_command* src)
{
    dst->data = src->data;
    dst->n_data = src->n_data;
    cb_message_type_id_copy(&dst->message_type_id, &src->message_type_id);

    src->data = NULL;
    src->n_data = 0;

    return dst;
}

cb_command cb_command_from_proto(ProtobufCebusCommand proto)
{
    cb_command command;
    const ProtobufCebusMessageDescriptor* cebus_descriptor = proto.base.descriptor;
    const ProtobufCMessageDescriptor* descriptor = proto.base.message->descriptor;

    cb_message_type_id_set(&command.message_type_id, "%s.%s", cebus_descriptor->namespace_name, descriptor->name);
    command.data = cb_pack_message(proto.base.message , &command.n_data);
    return command;
}

void cb_command_free(cb_command* command)
{
    free(command->data);
    command->n_data = 0;
}
