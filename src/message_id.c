#include "cebus/message_id.h"

#include "cebus/alloc.h"

#include <stdlib.h>
#include <string.h>

static Uuid* uuid_proto_new(const uuid_t uuid)
{
    Uuid* proto = cebus_alloc(sizeof *proto);
    uuid__init(proto);

    memcpy(&proto->hi, uuid, 8);
    memcpy(&proto->lo, uuid + 8, 8);

    return proto;
}

static void uuid_proto_free(Uuid* uuid)
{
    free(uuid);
}

void message_id_next(message_id* message_id)
{
    uuid_generate_time(message_id->value);
}

MessageId* message_id_proto_new(const message_id* message_id)
{
    MessageId* proto = cebus_alloc(sizeof *proto);
    message_id__init(proto);

    proto->value = uuid_proto_new(message_id->value);
    return proto;
}

void message_id_proto_free(MessageId* proto)
{
    uuid_proto_free(proto->value);
    free(proto);
}
