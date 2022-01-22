#pragma once

#include "cebus/cebus_bool.h"

#include <stddef.h>
#include <stdint.h>

#include <protobuf-c/protobuf-c.h>

#define CB_BINDING_KEY_ALL "*"
#define CB_BINDING_KEY_EMPTY "#"

typedef struct cb_binding_key_fragment
{
    const char* value;
} cb_binding_key_fragment;

typedef struct cb_binding_key
{
    char** parts;
    size_t n_parts;
} cb_binding_key;

typedef struct cb_binding_key_builder
{
    char** parts;
    size_t capacity;

    size_t n;
} cb_binding_key_builder;

/// default-initialize a new binding_key
void cb_binding_key_init(cb_binding_key* key);

/// Initialize a binding_key from a list of `count` fragments.
/// This function will do a copy of all the fragments pointed by `fragments`
cb_binding_key cb_binding_key_from_fragments(const char** fragments, size_t count);

/// Initialize a binding_key from a list of `count` fragments.
/// This function will "move" the fragments pointed by `fragments`
cb_binding_key cb_binding_key_from_fragments_raw(char** fragments, size_t count);

/// Return the number of fragments that constitute the `key`
size_t cb_binding_key_fragment_count(cb_binding_key key);

/// Return the string corresponding to the `key`.
/// Memory: This is the responsability of the user to free the underlying memory allocated
/// by this function
char* cb_binding_key_str(cb_binding_key key);

/// Free the memory allocated by the binding key pointed by `key`
void cb_binding_key_free(cb_binding_key* key);

/// Return a the fragment at `index`
cb_binding_key_fragment cb_binding_key_get_fragment(cb_binding_key key, size_t index);

cb_binding_key_builder cb_binding_key_builder_with_capacity(size_t count);
cebus_bool cb_binding_key_builder_add_bool(cb_binding_key_builder* builder, protobuf_c_boolean val);
cebus_bool cb_binding_key_builder_add_float(cb_binding_key_builder* builder, float val);
cebus_bool cb_binding_key_builder_add_double(cb_binding_key_builder* builder, double val);
cebus_bool cb_binding_key_builder_add_u32(cb_binding_key_builder* builder, uint32_t val);
cebus_bool cb_binding_key_builder_add_u64(cb_binding_key_builder* builder, uint64_t val);
cebus_bool cb_binding_key_builder_add_i32(cb_binding_key_builder* builder, int32_t val);
cebus_bool cb_binding_key_builder_add_i64(cb_binding_key_builder* builder, int64_t val);
cebus_bool cb_binding_key_builder_add_str(cb_binding_key_builder* builder, const char* s);
cb_binding_key cb_binding_key_build(cb_binding_key_builder* builder);
