#pragma once

#include "cebus/cebus_bool.h"
#include "cebus/collection/hasher.h"

#include <stddef.h>
#include <stdint.h>

typedef void * cb_hash_key_t;
typedef void * cb_hash_value_t;
typedef unsigned int cb_hash_t;

typedef struct cb_hash_bucket
{
    /// The hash value of this bucket
    cb_hash_t hash;

    /// The key of this bucket
    cb_hash_key_t key;

    /// The value of this bucket
    cb_hash_value_t value;
} cb_hash_bucket;

/// The hashing function to use to hash the keys
typedef void (*cb_hash_func)(cb_hasher*, cb_hash_key_t);

/// The function to use to compare equality between keys
typedef cebus_bool (*cb_hash_eq)(cb_hash_key_t, cb_hash_key_t);

/// The function to use to iterator over the hashmap entries
typedef void (*cb_hash_iter)(const cb_hash_key_t, const cb_hash_value_t, void* user);

/// The function to use to delete an entry
typedef void (*cb_hash_dtor)(cb_hash_key_t, cb_hash_value_t, void* user);

/// A hash map with linear probing collision resolution
typedef struct cb_hash_map
{
    cb_hash_func   hash_func;
    cb_hash_eq     key_eq;

    size_t shift;
    cb_hash_t mask;

    cb_hash_bucket* buckets;
    cb_hasher hasher;

    /// The size of the underlying array of the hash table
    size_t size;

    /// The actual number of entries in the hash table
    size_t n_entries;
} cb_hash_map;

/// Create a new hashmap with `hash_func` as the hashing function and `key_eq` as the key comparator
cb_hash_map *cb_hash_map_new(cb_hash_func hash_func, cb_hash_eq key_eq);

/// Get the value from the `map` corresponding to the `key` or `NULL` if not present
cb_hash_value_t cb_hash_get(cb_hash_map* map, const cb_hash_key_t key);

/// Insert a new pair of (`key`, `value`) to the `map` and return the previous existing value for the `key`
/// or `NULL` it the key was not present in the `map`
cb_hash_value_t cb_hash_insert(cb_hash_map* map, const cb_hash_key_t key, const cb_hash_value_t value);

/// Remove all the keys from the `map` and invoke the `destructor` function with `user` data on every entry if not NULL
void cb_hash_clear(cb_hash_map* map, cb_hash_dtor destructor, void* user);

/// Remove a `key` from the `map` and return the corresponding value or `NULL` it the key was not present
/// in the `map`
cb_hash_value_t cb_hash_remove(cb_hash_map* map, const cb_hash_key_t key);

/// Return the number of entries in the `map` 
size_t cb_hash_len(const cb_hash_map* map);

/// Iterate over every entry of the `map` and call the `func` callback for each entry
void cb_hash_foreach(const cb_hash_map* map, cb_hash_iter func, void* user);
//
/// Free the underlying memory owned by the `map` and call the `destructor` function with the `user`-provided data
/// on every entry of the map
void cb_hash_map_free(cb_hash_map* map, cb_hash_dtor destructor, void* user);

/// Hash a single `uint64_t` key
void cb_hash_u64(cb_hasher* hasher, cb_hash_key_t key);

/// Compare two `uint64_t` keys
cebus_bool cb_hash_eq_u64(cb_hash_key_t lhs, cb_hash_key_t rhs);
