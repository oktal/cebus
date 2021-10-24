#pragma once

#include <stddef.h>

#include "binding_key.pb-c.h"

typedef struct binding_key_fragment {
    const char* value;
} binding_key_fragment;

typedef struct binding_key {
    BindingKey proto;
} binding_key;

binding_key *binding_key_new(const char** fragments, size_t count);
size_t binding_key_fragment_count(const binding_key* key);
binding_key_fragment binding_key_get_fragment(const binding_key* key, size_t index);
