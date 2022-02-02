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

void cb_message_id_hash(cb_hasher* hasher, cb_hash_key_t message_id_key)
{
    const cb_message_id* message_id = (cb_message_id *)message_id_key;
    char buf[36 + 1]; // +1 for '\0'
    memset(buf, 0, sizeof(buf));

    cb_uuid_print(&message_id->value, buf, sizeof(buf));
    cb_hasher_write_str(hasher, buf);
}

cebus_bool cb_message_id_hash_eq(cb_hash_key_t lhs, cb_hash_key_t rhs)
{
    const cb_message_id* message_id_lhs = (const cb_message_id *)lhs;
    const cb_message_id* message_id_rhs = (const cb_message_id *)rhs;

    return cb_uuid_eq(&message_id_lhs->value, &message_id_rhs->value);
}
