#include "cebus/message_type_id.h"

#include "cebus/alloc.h"

#include <stdlib.h>
#include <string.h>

void cb_message_type_id_set(cb_message_type_id* type_id, const char* value)
{
    strncpy(type_id->value, value, CEBUS_STR_MAX);
}

void cb_message_type_id_copy(cb_message_type_id* dst, const cb_message_type_id* src)
{
    cb_message_type_id_set(dst, src->value);
}

cebus_bool cb_message_type_id_eq_str(const cb_message_type_id* type_id, const char* value)
{
    return cebus_bool_from_int(strncmp(type_id->value, value, CEBUS_STR_MAX) == 0);
}

cebus_bool cb_message_type_id_eq(const cb_message_type_id* lhs, const cb_message_type_id* rhs)
{
    return cebus_bool_from_int(strncmp(lhs->value, rhs->value, CEBUS_STR_MAX) == 0);
}

void cb_message_type_id_from_proto(cb_message_type_id* type_id, const MessageTypeId* proto)
{
    cb_message_type_id_set(type_id, proto->full_name);
}

MessageTypeId* cb_message_type_id_proto_new(const cb_message_type_id* type_id)
{
    MessageTypeId* proto = cb_new(MessageTypeId, 1);
    message_type_id__init(proto);
    proto->full_name = cb_strdup(type_id->value);
    return proto;
}

void cb_message_type_id_proto_free(MessageTypeId* proto)
{
    free(proto->full_name);
    free(proto);
}

void cb_message_type_id_hash(cb_hasher* hasher, cb_hash_key_t message_type_id_key)
{
    const cb_message_type_id* type_id = (cb_message_type_id *)message_type_id_key;
    cb_hasher_write_str(hasher, type_id->value);
}

cebus_bool cb_message_type_id_hash_eq(cb_hash_key_t lhs, cb_hash_key_t rhs)
{
    const cb_message_type_id* m0 = (cb_message_type_id *) lhs;
    const cb_message_type_id* m1 = (cb_message_type_id *) rhs;
    return cb_message_type_id_eq(m0, m1);
}
