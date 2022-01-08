#include "cebus/message_type_id.h"

#include "cebus/alloc.h"

#include <stdlib.h>
#include <string.h>

void message_type_id_set(message_type_id* type_id, const char* value)
{
    strncpy(type_id->value, value, CEBUS_STR_MAX);
}

cebus_bool message_type_id_equal(const message_type_id* type_id, const char* value)
{
    return cebus_bool_from_int(!strncmp(type_id->value, value, CEBUS_STR_MAX));
}

MessageTypeId* message_type_id_proto_new(const message_type_id* type_id)
{
    MessageTypeId* proto = cebus_alloc(sizeof *proto);
    message_type_id__init(proto);
    proto->full_name = cebus_strdup(type_id->value);

    return proto;
}

void message_type_id_proto_free(MessageTypeId* proto)
{
    free(proto->full_name);
    free(proto);
}
