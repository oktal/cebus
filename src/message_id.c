#include "cebus/message_id.h"

#include "cebus/alloc.h"

#include <stdlib.h>
#include <string.h>

static Bcl__Guid* cb_guid_proto_new(const cb_uuid_t* uuid)
{
    const size_t BCL_GUID_BYTES = 8;

    Bcl__Guid* proto = cb_new(Bcl__Guid, 1);
    bcl__guid__init(proto);

    memcpy(&proto->hi, uuid->bits, BCL_GUID_BYTES);
    memcpy(&proto->lo, uuid->bits + BCL_GUID_BYTES, BCL_GUID_BYTES);

    return proto;
}

static void cb_guid_proto_free(Bcl__Guid* guid)
{
    free(guid);
}

void cb_message_id_next(cb_message_id* message_id, cb_time_uuid_gen* gen)
{
    cb_uuid_generate_time(gen, &message_id->value);
}

MessageId* cb_message_id_proto_new(const cb_message_id* message_id)
{
    MessageId* proto = cb_new(MessageId, 1);
    message_id__init(proto);

    proto->value = cb_guid_proto_new(&message_id->value);
    return proto;
}

void cb_message_id_proto_free(MessageId* proto)
{
    cb_guid_proto_free(proto->value);
    free(proto);
}
