#include "cebus/binding_key.h"

#include "cebus/alloc.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static cb_binding_key_builder cb_binding_key_builder_new(char** parts, size_t count)
{
    const cb_binding_key_builder builder = { parts, count, 0 };
    return builder;
}

static cebus_bool cb_binding_key_builder_add(cb_binding_key_builder* builder, const char* fmt, ...)
{
    if (builder->n >= builder->capacity)
    {
        return cebus_false;
    }
    else
    {
        char buf[32];
        va_list args;

        memset(buf, 0, sizeof(buf));
        va_start(args, fmt);
        vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);

        builder->parts[builder->n] = cb_strdup(buf);
        builder->n += 1;

        return cebus_true;
    }
}

void cb_binding_key_init(cb_binding_key* key)
{
    static const cb_binding_key default_value = { NULL, 0 };
    *key = default_value;
}

void cb_binding_key_copy(cb_binding_key* dst, const cb_binding_key* src)
{
    if (src->parts == NULL)
    {
        dst->parts = NULL;
        dst->n_parts = 0;
    }
    else
    {
        dst->parts = cb_new(char *, src->n_parts);
        dst->n_parts = src->n_parts;

        {
            size_t i;
            for (i = 0; i < src->n_parts; ++i)
            {
                printf("Copying binding_key part[%zu] = %s\n", i, src->parts[i]);
                dst->parts[i] = cb_strdup(src->parts[i]);
            }
        }
    }
}

cb_binding_key cb_binding_key_from_fragments(const char** fragments, size_t count)
{
    cb_binding_key key;

    key.parts = cb_new(char *, count);
    key.n_parts = count;

    {
        size_t i;
        for (i = 0; i < count; ++i)
        {
            key.parts[i] = cb_strdup(fragments[i]);
        }
    }

    return key;
}

cb_binding_key cb_binding_key_from_fragments_raw(char** fragments, size_t count)
{
    cb_binding_key key;
    key.parts = fragments;
    key.n_parts = count;

    return key;
}

cebus_bool cb_binding_key_is_empty(cb_binding_key key)
{
    return cebus_bool_from_int(
            key.parts == NULL || (key.n_parts == 1 && strcmp(key.parts[0], CB_BINDING_KEY_TOKEN_SHARP) == 0));
}

cebus_bool cb_binding_key_is_sharp(cb_binding_key key, size_t index)
{
    return cb_binding_key_fragment_is_sharp(cb_binding_key_get_fragment(key, index));
}

cebus_bool cb_binding_key_is_star(cb_binding_key key, size_t index)
{
    return cb_binding_key_fragment_is_star(cb_binding_key_get_fragment(key, index));
}

size_t cb_binding_key_fragment_count(cb_binding_key key)
{
    return key.n_parts;
}

char* cb_binding_key_str(cb_binding_key key)
{
    size_t size = 0, i = 0;
    char* out_str = NULL, *p = NULL;

    if (key.parts == NULL)
        return cb_strdup(CB_BINDING_KEY_EMPTY);

    // Compute the size of the key by summing the size of all parts
    for (i = 0; i < key.n_parts; ++i)
    {
        size += strlen(key.parts[i]);
    }
    // Add the size of the dots '.'
    size += key.n_parts;

    // Allocate the resulting string
    out_str = cb_new(char, size);

    // Copy the parts
    p = out_str;
    for (i = 0; i < key.n_parts; ++i)
    {
        const char* part = key.parts[i];
        if (i > 0)
            *p++ = '.';

        // This should be safe as we pre-computed the size based on the size of all the parts
        strcpy(p, part);
        p += strlen(part);
    }

    return out_str;
}

void cb_binding_key_from_proto(cb_binding_key* binding_key, const BindingKey* proto)
{
    binding_key->parts = cb_new(char *, proto->n_parts);
    binding_key->n_parts = proto->n_parts;

    {
        size_t i;
        for (i = 0; i < proto->n_parts; ++i)
        {
            binding_key->parts[i] = cb_strdup(proto->parts[i]);
        }
    }
}

BindingKey* cb_binding_key_proto_new(const cb_binding_key* binding_key)
{
    BindingKey* message = cb_new(BindingKey, 1);
    binding_key__init(message);

    if (binding_key->parts == NULL)
    {
        message->parts = cb_new(char *, 1);
        message->n_parts = 1;

        message->parts[0] = CB_BINDING_KEY_TOKEN_SHARP;
    }
    else
    {
        message->parts = cb_new(char *, binding_key->n_parts);
        message->n_parts = binding_key->n_parts;

        {
            size_t i;
            for (i = 0; i < binding_key->n_parts; ++i)
            {
                message->parts[i] = cb_strdup(binding_key->parts[i]);
            }
        }
    }

    return message;
}

void cb_binding_key_proto_free(BindingKey* message)
{
    size_t i;
    for (i = 0; i < message->n_parts; ++i)
    {
        free(message->parts[i]);
    }

    free(message->parts);
}

cb_binding_key cb_binding_key_from_message(const ProtobufCMessage* message, const ProtobufCebusMessageDescriptor* descriptor)
{
    if (descriptor->routing_fields == NULL)
    {
        cb_binding_key binding_key;
        cb_binding_key_init(&binding_key);
        return binding_key;
    }
    else
    {
        cb_binding_key_builder builder = cb_binding_key_builder_with_capacity(descriptor->n_routing_fields);
        size_t i;

        for (i = 0; i < descriptor->n_routing_fields; ++i)
        {
            const ProtobufCebusRoutingFieldDescriptor* routing_field_descriptor = descriptor->routing_fields + i;
            const ProtobufCFieldDescriptor* field_descriptor = routing_field_descriptor->descriptor;
            const void* member = ((const char *) message) + field_descriptor->offset;

            if (field_descriptor->label == PROTOBUF_C_LABEL_OPTIONAL)
            {
                const void *qmember =
                    ((const char *) message) + field_descriptor->quantifier_offset;
                const protobuf_c_boolean has = *(const protobuf_c_boolean *) qmember;

                if (!has)
                {
                    cb_binding_key_builder_add_str(&builder, CB_BINDING_KEY_ALL);
                    continue;
                }
            }

            switch (field_descriptor->type)
            {
                case PROTOBUF_C_TYPE_BOOL:
                    cb_binding_key_builder_add_bool(&builder, cebus_bool_from_int(*(protobuf_c_boolean *) member));
                    break;
                case PROTOBUF_C_TYPE_SFIXED32:
                case PROTOBUF_C_TYPE_SINT32:
                case PROTOBUF_C_TYPE_INT32:
                    cb_binding_key_builder_add_i32(&builder, *(int32_t *) member);
                    break;
                case PROTOBUF_C_TYPE_FIXED32:
                case PROTOBUF_C_TYPE_UINT32:
                    cb_binding_key_builder_add_u32(&builder, *(uint32_t *) member);
                    break;
                case PROTOBUF_C_TYPE_SFIXED64:
                case PROTOBUF_C_TYPE_SINT64:
                case PROTOBUF_C_TYPE_INT64:
                    cb_binding_key_builder_add_i64(&builder, *(int32_t *) member);
                    break;
                case PROTOBUF_C_TYPE_FIXED64:
                case PROTOBUF_C_TYPE_UINT64:
                    cb_binding_key_builder_add_u64(&builder, *(uint64_t *) member);
                    break;
                case PROTOBUF_C_TYPE_FLOAT:
                    cb_binding_key_builder_add_float(&builder, *(float *) member);
                    break;
                case PROTOBUF_C_TYPE_DOUBLE:
                    cb_binding_key_builder_add_double(&builder, *(double *) member);
                    break;
                case PROTOBUF_C_TYPE_STRING:
                    cb_binding_key_builder_add_str(&builder, *(char *const *)member);
                    break;
                default:
                    assert(false && "Unsupported type for binding key");
            }

        }

        return cb_binding_key_build(CB_MOVE(&builder));
    }
}

cb_binding_key cb_binding_key_from_str(const char* fmt, ...)
{
    size_t count = 0, len = 0;
    char* part;
    char *buf, *buf1;
    va_list args1, args2;

    va_start(args1, fmt);
    va_copy(args2, args1);

    len = vsnprintf(NULL, 0, fmt, args1) + 1;
    buf = cb_alloc(len);
    vsnprintf(buf, len, fmt, args2);

    va_end(args1);
    va_end(args2);

    buf1 = cb_strdup(buf);

    while ((part = strsep(&buf, ".")) != NULL)
    {
        ++count;
    }

    cb_binding_key key;
    {
        cb_binding_key_builder builder = cb_binding_key_builder_with_capacity(count);
        while ((part = strsep(&buf1, ".")) != NULL)
        {
            cb_binding_key_builder_add_str(&builder, part);
        }

        key = cb_binding_key_build(&builder);
    }

    free(buf);
    free(buf1);

    return key;
}

void cb_binding_key_free(cb_binding_key* key)
{
    size_t i;
    for (i = 0; i < key->n_parts; ++i)
        free(key->parts[i]);

    key->n_parts = 0;
}

cb_binding_key_fragment cb_binding_key_get_fragment(cb_binding_key key, size_t index)
{
    cb_binding_key_fragment fragment;

    if (index >= key.n_parts)
        fragment.value = NULL;
    else
        fragment.value = key.parts[index];

    return fragment;
}

cb_binding_key_fragment cb_binding_key_fragment_clone(cb_binding_key_fragment fragment)
{
    cb_binding_key_fragment clone;

    if (fragment.value == NULL)
        clone.value = NULL;
    else
        clone.value = cb_strdup(fragment.value);

    return clone;
}

cebus_bool cb_binding_key_fragment_is_empty(cb_binding_key_fragment fragment)
{
    return cebus_bool_from_int(fragment.value == NULL);
}

cebus_bool cb_binding_key_fragment_is_sharp(cb_binding_key_fragment fragment)
{
    if (cb_binding_key_fragment_is_empty(fragment))
        return cebus_false;

    return cebus_bool_from_int(strcmp(fragment.value, CB_BINDING_KEY_TOKEN_SHARP) == 0);
}

cebus_bool cb_binding_key_fragment_is_star(cb_binding_key_fragment fragment)
{
    if (cb_binding_key_fragment_is_empty(fragment))
        return cebus_false;

    return cebus_bool_from_int(strcmp(fragment.value, CB_BINDING_KEY_TOKEN_STAR) == 0);
}

cebus_bool cb_binding_key_fragment_eq(cb_binding_key_fragment lhs, cb_binding_key_fragment rhs)
{
    if (cb_binding_key_fragment_is_empty(lhs) && cb_binding_key_fragment_is_empty(rhs))
        return cebus_true;

    if (cb_binding_key_fragment_is_empty(lhs) || cb_binding_key_fragment_is_empty(rhs))
        return cebus_false;

    return cebus_bool_from_int(strcmp(lhs.value, rhs.value) == 0);
}

void cb_binding_key_fragment_free(cb_binding_key_fragment *fragment)
{
    free(fragment->value);
}

cb_binding_key_builder cb_binding_key_builder_with_capacity(size_t count)
{
    return cb_binding_key_builder_new(cb_new(char *, count), count);
}

cebus_bool cb_binding_key_builder_add_bool(cb_binding_key_builder* builder, cebus_bool val)
{
    return cb_binding_key_builder_add(builder, "%d", !!val);
}

cebus_bool cb_binding_key_builder_add_float(cb_binding_key_builder* builder, float val)
{
    return cb_binding_key_builder_add(builder, "%f", val);
}

cebus_bool cb_binding_key_builder_add_double(cb_binding_key_builder* builder, double val)
{
    return cb_binding_key_builder_add(builder, "%lf", val);
}

cebus_bool cb_binding_key_builder_add_u32(cb_binding_key_builder* builder, uint32_t val)
{
    return cb_binding_key_builder_add(builder, "%"PRIu32, val);
}

cebus_bool cb_binding_key_builder_add_u64(cb_binding_key_builder* builder, uint64_t val)
{
    return cb_binding_key_builder_add(builder, "%"PRIu64, val);
}

cebus_bool cb_binding_key_builder_add_i32(cb_binding_key_builder* builder, int32_t val)
{
    return cb_binding_key_builder_add(builder, "%"PRIi32, val);
}

cebus_bool cb_binding_key_builder_add_i64(cb_binding_key_builder* builder, int64_t val)
{
    return cb_binding_key_builder_add(builder, "%"PRIi64, val);
}

cebus_bool cb_binding_key_builder_add_str(cb_binding_key_builder* builder, const char* s)
{
    if (builder->n >= builder->capacity)
        return cebus_false;

    builder->parts[builder->n] = cb_strdup(s);
    builder->n += 1;

    return cebus_true;
}

cb_binding_key cb_binding_key_build(cb_binding_key_builder* builder)
{
    const cb_binding_key key = cb_binding_key_from_fragments_raw(builder->parts, builder->n);

    builder->parts = NULL;
    builder->capacity = builder->n = 0;

    return key;
}
