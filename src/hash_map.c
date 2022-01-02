#include "cebus/collection/hash_map.h"

#include "cebus/alloc.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CB_HASH_INIT_SHIFT 3
#define CB_HASH_DEBUG 0

static void cb_hash_bucket_reset(cb_hash_bucket* bucket)
{
    bucket->hash = (cb_hash_t) 0;
    bucket->key = bucket->value = NULL;
}

static void cb_hash_bucket_tombstone(cb_hash_bucket* bucket)
{
    bucket->hash = (cb_hash_t) 0;
    bucket->value = NULL;
}

static cebus_bool cb_hash_bucket_empty(const cb_hash_bucket* bucket)
{
    return cebus_bool_from_int(bucket->key == NULL);
}

static void cb_hash_set_size(cb_hash_map* map, size_t shift)
{
    map->shift = shift;
    map->size = 1 << shift;
    map->mask = map->size - 1;
}

static void cb_hash_alloc_storage(cb_hash_map* map, size_t shift)
{
    size_t i;
    cb_hash_set_size(map, shift);

    map->buckets = cb_alloc(cb_hash_bucket, map->size);

    for (i = 0; i < map->size; ++i)
    {
        cb_hash_bucket* bucket = &map->buckets[i];
        cb_hash_bucket_reset(bucket);
    }
}

static cb_hash_bucket* cb_hash_lookup_bucket(cb_hash_map* map, cb_hash_key_t key, cb_hash_t hash)
{
    size_t bucket_index = hash & map->mask;
    cb_hash_key_t bucket_key;
    while ((bucket_key = map->buckets[bucket_index].key) != NULL)
    {
        cb_hash_t bucket_hash = map->buckets[bucket_index].hash;
        if (bucket_hash == hash)
        {
            if (map->key_eq(bucket_key, key) == cebus_true)
                break;
        }

        bucket_index += 1;
    }

    return &map->buckets[bucket_index];
}

static cb_hash_bucket* cb_hash_lookup(cb_hash_map* map, cb_hash_key_t key, cb_hash_t* out_hash)
{
    cb_hash_t hash;
    cb_hasher* hasher = &map->hasher;

    cb_hasher_reset(hasher);
    map->hash_func(hasher, key);
    hash = cb_hasher_finish(hasher);

    if (out_hash != NULL)
        *out_hash = hash;

    return cb_hash_lookup_bucket(map, key, hash);
}

static void cb_hash_resize(cb_hash_map* map, size_t shift)
{
    #if DEBUG
        printf("Resizing with new shift %lu\n", shift);
    #endif

    cb_hasher* hasher = &map->hasher;
    cb_hash_bucket* old_buckets = map->buckets;
    size_t old_size = map->size;
    size_t i;

    cb_hash_map new_map;
    cb_hasher_init(&new_map.hasher, hasher->k0, hasher->k1);
    cb_hash_alloc_storage(&new_map, shift);
    for (i = 0; i < old_size; ++i)
    {
        const cb_hash_bucket* old_bucket = &old_buckets[i];
        cb_hash_bucket* new_bucket = cb_hash_lookup_bucket(&new_map, old_bucket->key, old_bucket->hash);
        *new_bucket = *old_bucket;
    }

    map->shift = new_map.shift;
    map->mask =new_map.mask;
    map->buckets = new_map.buckets;
    map->size = new_map.size;

    free(old_buckets);
}

static void cb_hash_grow_if_needed(cb_hash_map* map)
{
    const size_t n_entries = map->n_entries;
    if (n_entries >= (map->size * 3) / 4)
        cb_hash_resize(map, map->shift + 1);
}

cb_hash_map* cb_hash_map_new(cb_hash_func hash_func, cb_hash_eq key_eq)
{
    cb_hash_map* map = cb_alloc(cb_hash_map, 1);
    map->hash_func = hash_func;
    map->key_eq = key_eq;

    map->size = 0;
    map->n_entries = 0;

    cb_hash_alloc_storage(map, CB_HASH_INIT_SHIFT);
    cb_hasher_init_random(&map->hasher);

    return map;
}

void cb_hash_map_free(cb_hash_map* map)
{
    free(map->buckets);
    free(map);
}

cb_hash_value_t cb_hash_get(cb_hash_map* map, const cb_hash_key_t key)
{
    cb_hash_bucket* bucket = cb_hash_lookup(map, key, NULL);
    return bucket->value;
}

cb_hash_value_t cb_hash_insert(cb_hash_map* map, const cb_hash_key_t key, const cb_hash_value_t value)
{
    cb_hash_t hash;
    cb_hash_bucket* bucket = cb_hash_lookup(map, key, &hash);

    cb_hash_key_t bucket_key = bucket->key;
    cb_hash_value_t bucket_value = bucket->value;

    #if CB_HASH_DEBUG
      const ptrdiff_t index = bucket - map->buckets;
      printf("Inserting at index %lu with hash %ull\n", index, hash);
    #endif

    if (bucket_key == NULL)
    {
        bucket->hash = hash;
        bucket->key = key;
        bucket->value = value;
        map->n_entries++;
    }
    else
    {
        bucket->value = value;
    }

    cb_hash_grow_if_needed(map);

    #if CB_HASH_DEBUG
      cb_hash_print(map);
    #endif
            
    return bucket_value;
}

cb_hash_value_t cb_hash_remove(cb_hash_map* map, const cb_hash_key_t key)
{
    cb_hash_bucket* bucket = cb_hash_lookup(map, key, NULL);
    cb_hash_value_t value = bucket->value;

    if (value != NULL)
    {
        cb_hash_bucket_tombstone(bucket);
        map->n_entries--;
    }

    return value;
}

size_t cb_hash_len(const cb_hash_map* map)
{
    return map->n_entries;
}

void cb_hash_foreach(const cb_hash_map* map, cb_hash_iter func)
{
    size_t i;
    for (i = 0; i < map->size; ++i)
    {
        const cb_hash_bucket* bucket = &map->buckets[i];
        if (cb_hash_bucket_empty(bucket) == cebus_false)
            func(bucket->key, bucket->value);
    }
}

void cb_hash_u64(cb_hasher* hasher, cb_hash_key_t key)
{
    cb_hasher_write_u64(hasher, *(uint64_t *) key);
}

cebus_bool cb_hash_eq_u64(cb_hash_key_t lhs, cb_hash_key_t rhs)
{
    return cebus_bool_from_int(*(uint64_t *)lhs == *(uint64_t *)rhs);
}
