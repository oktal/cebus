#pragma once

#include <stdint.h>
#include <stddef.h>

typedef struct cb_hasher {
    uint64_t k0;
    uint64_t k1;

    size_t len;

    uint64_t v0;
    uint64_t v1;
    uint64_t v2;
    uint64_t v3;

    uint64_t tail;
    uint64_t n_tail;
} cb_hasher;

#define CB_HASHER_OVERLOADS    \
    CB_HASHER_O(uint8_t , u8)  \
    CB_HASHER_O(uint16_t, u16) \
    CB_HASHER_O(uint32_t, u32) \
    CB_HASHER_O(uint64_t, u64) \

void cb_hasher_init(cb_hasher* hasher, uint64_t k0, uint64_t k1);
void cb_hasher_init_random(cb_hasher* hasher);
void cb_hasher_write(cb_hasher* hasher, const void* buf, size_t len);

#define CB_HASHER_O(T, suffix)                                   \
    void cb_hasher_write_## suffix(cb_hasher* hasher, T val);    \
    uint64_t cb_hasher_hash_## suffix(cb_hasher* hasher, T val);
  CB_HASHER_OVERLOADS
#undef CB_HASHER_O

void cb_hasher_write_str(cb_hasher* hasher, const char* str);

uint64_t cb_hasher_finish(cb_hasher* hasher);
void cb_hasher_reset(cb_hasher* hasher);
