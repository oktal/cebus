#pragma once

#include "cebus/cebus_bool.h"

#include <stddef.h>
#include <stdint.h>

#include <protobuf-c/protobuf-c.h>

#define CB_BINDING_KEY_ALL "*"
#define CB_BINDING_KEY_EMPTY "#"

#define CB_BINDING_KEY_TOKEN_STAR "*"
#define CB_BINDING_KEY_TOKEN_SHARP "#"

typedef struct cb_binding_key_fragment
{
    char* value;
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

/// Copy a `cb_binding_key` from `src` to `dst`
void cb_binding_key_copy(cb_binding_key* dst, const cb_binding_key* src);

/// Initialize a binding_key from a list of `count` fragments.
/// This function will do a copy of all the fragments pointed by `fragments`
cb_binding_key cb_binding_key_from_fragments(const char** fragments, size_t count);

/// Initialize a binding_key from a list of `count` fragments.
/// This function will "move" the fragments pointed by `fragments`
cb_binding_key cb_binding_key_from_fragments_raw(char** fragments, size_t count);

/// Return `cebus_true` if the given `key` is empty or `cebus_false` otherwise
/// An empty binding key is whether `NULL` or `*`
cebus_bool cb_binding_key_is_empty(cb_binding_key key);

/// Return `cebus_true` if the fragment of `key` at index `index` is a sharp token
/// or `cebus_false` otherwise
/// Behavior is undefined if `index` is out of range
cebus_bool cb_binding_key_is_sharp(cb_binding_key key, size_t index);

/// Return `cebus_true` if the fragment of `key` at index `index` is a star token
/// or `cebus_false` otherwise
/// Behavior is undefined if `index` is out of range
cebus_bool cb_binding_key_is_star(cb_binding_key key, size_t index);

/// Return the number of fragments that constitute the `key`
size_t cb_binding_key_fragment_count(cb_binding_key key);

/// Create a `binding_key` from a string representation of the form `part1.part2.partn`
cb_binding_key cb_binding_key_from_str(const char* str, ...);

/// Return the string corresponding to the `key`.
/// Memory: This is the responsability of the user to free the underlying memory allocated
/// by this function
char* cb_binding_key_str(cb_binding_key key);

/// Free the memory allocated by the binding key pointed by `key`
void cb_binding_key_free(cb_binding_key* key);

/// Return the fragment of the binding key `key` at index `index`
/// The fragment returned by this function does not own any memory.
// Use `cb_binding_key_fragment_clone` to create an owning version of a fragment
cb_binding_key_fragment cb_binding_key_get_fragment(cb_binding_key key, size_t index);

/// Return a clone of the given `fragment`
/// Memory: the memory allocated by this function must be free'd by calling `cb_binding_key_fragment_free`
cb_binding_key_fragment cb_binding_key_fragment_clone(cb_binding_key_fragment fragment);

/// Return `cebus_true` if the given `fragment` is empty or `cebus_false` otherwise
cebus_bool cb_binding_key_fragment_is_empty(cb_binding_key_fragment fragment);

/// Return `cebus_true` if the given `fragment` is a sharp token or `cebus_false` otherwise
cebus_bool cb_binding_key_fragment_is_sharp(cb_binding_key_fragment fragment);

/// Return `cebus_true` if the given `fragment` is a star token or `cebus_false` otherwise
cebus_bool cb_binding_key_fragment_is_star(cb_binding_key_fragment fragment);

/// Return `cebus_true` both fragments are equal or `cebus_false` otherwise
cebus_bool cb_binding_key_fragment_eq(cb_binding_key_fragment lhs, cb_binding_key_fragment rhs);

/// Free the memory that might be owned by this `fragment`
/// The behavior is undefined if the fragment has not been created by a call to a function that
/// previously allocated memory like `cb_binding_key_fragment_clone`
void cb_binding_key_fragment_free(cb_binding_key_fragment* fragment);

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
