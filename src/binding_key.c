#include "cebus/binding_key.h"

#include "cebus/alloc.h"
#include "cebus/iter_utils.h"

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

cb_binding_key cb_binding_key_from_fragments(const char** fragments, size_t count)
{
    cb_binding_key key;

    key.parts = cb_new(char *, count);
    key.n_parts = count;

    cebus_for_range(i, 0, count, {
        key.parts[i] = cb_strdup(fragments[i]);
    });

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

cebus_bool cb_binding_key_builder_add_bool(cb_binding_key_builder* builder, protobuf_c_boolean val)
{
    return cb_binding_key_builder_add(builder, "%d", val);
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
