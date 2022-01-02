#include "cebus/collection/hasher.h"
#include "cebus/cebus_bool.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#if defined(__linux__)
  #include <sys/random.h>
#else
  #error "Platform not supported"
#endif
  
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define ROT_LEFT(x, b) (uint64_t)(((x) << (b)) | ((x) >> (64 - (b))))

#define U8TO16_LE(p, start) \
    (((uint16_t) p[start + 0]) << 0   | (((uint16_t) p[start + 1]) << 8))

#define U8TO32_LE(p, start) \
    ((uint32_t) U8TO16_LE(p, start)) | (((uint32_t) U8TO16_LE(p, start + 2)) << 16)

#define U8TO64_LE(p, start) \
    ((uint64_t) U8TO32_LE(p, start)) | (((uint64_t) U8TO32_LE(p, start + 4)) << 32)

#define KEY_PART_LEN sizeof(uint64_t)
#define KEY_LEN KEY_PART_LEN * 2

static void compress(cb_hasher* hasher)
{
    uint64_t v0 = hasher->v0;
    uint64_t v1 = hasher->v1;
    uint64_t v2 = hasher->v2;
    uint64_t v3 = hasher->v3;

    v0 += v1;
    v1 = ROT_LEFT(v1, 13);
    v1 ^= v0;
    v0 = ROT_LEFT(v0, 32);
    v2 += v3;
    v3 = ROT_LEFT(v3, 16);
    v3 ^= v2;
    v0 += v3;
    v3 = ROT_LEFT(v3, 21);
    v3 ^= v0;
    v2 += v1;
    v1 = ROT_LEFT(v1, 17);
    v1 ^= v2;
    v2 = ROT_LEFT(v2, 32);

    hasher->v0 = v0;
    hasher->v1 = v1;
    hasher->v2 = v2;
    hasher->v3 = v3;
}

static void sip24_compress(cb_hasher* hasher)
{
    compress(hasher);
    compress(hasher);
}

static void sip24_finalize(cb_hasher* hasher)
{
    compress(hasher);
    compress(hasher);
    compress(hasher);
    compress(hasher);
}

static uint64_t u8to64_le(const uint8_t* buf, size_t start, size_t len)
{
    size_t i = 0;
    uint64_t out = 0;

    if (i + 3 < len) {
        out = U8TO32_LE(buf, start + i);
        i += 4;
    }

    if (i + 1 < len) {
        out |= (uint64_t) U8TO16_LE(buf, start + i) << (i * 8);
        i += 2;
    }

    if (i < len) {
        out |= (uint64_t) buf[start + i] << (i * 8);
        i += 1;
    }

    assert(i == len);
    return out;
}

static void set_key_part_from_bytes(const void* bytes, uint64_t *key, size_t n)
{
    memcpy(key, bytes + (n * KEY_PART_LEN), KEY_PART_LEN);
}

#if defined(__linux__)
static cebus_bool get_random_bytes(uint8_t* dst, size_t len)
{
    ssize_t res = getrandom(dst, len, GRND_NONBLOCK); 
    return cebus_bool_from_int(res == 0);
}
#endif

static void get_random_key(uint64_t* k0, uint64_t* k1)
{
    uint8_t bytes[KEY_LEN];
    get_random_bytes(bytes, KEY_LEN);

    set_key_part_from_bytes(bytes, k0, 0);
    set_key_part_from_bytes(bytes, k1, 1);
}

void cb_hasher_write(cb_hasher* hasher, const void* buf, size_t len)
{
    uint64_t needed = 0;
    size_t length, left, i;
    const uint8_t *bytes = (const uint8_t *)buf;

    hasher->len += len;

    // If we have a tail waiting, process it
    if (hasher->n_tail != 0) {
        needed = 8 - hasher->n_tail;
        hasher->tail |= u8to64_le(bytes, 0, MIN(len, needed)) << (8 * hasher->n_tail);
        if (len < needed) {
            hasher->n_tail += len;
            return;
        } else {
            hasher->v3 ^= hasher->tail;
            sip24_compress(hasher);
            hasher->v0 ^= hasher->tail;
            hasher->n_tail = 0;
        }
    }


    // Tail has been flushed, process input
    length = len - needed;
    left = length & 0x07; // % 8

    i = needed;
    while (i < length - left) {
        uint64_t mi = U8TO64_LE(bytes, i);
        hasher->v3 ^= mi;
        sip24_compress(hasher);
        hasher->v0 ^= mi;

        i += sizeof(uint64_t);
    }

    // Update tail if we have left-over bytes
    hasher->tail = u8to64_le(bytes, i, left);
    hasher->n_tail = left;
}

void cb_hasher_init(cb_hasher* hasher, uint64_t k0, uint64_t k1)
{
    hasher->k0 = k0;
    hasher->k1 = k1;

    hasher->len = 0;
    hasher->tail = hasher->n_tail = 0;
    hasher->v0 = hasher->v1 = hasher->v2 = hasher->v3 = 0;
    cb_hasher_reset(hasher);
}

void cb_hasher_init_random(cb_hasher* hasher)
{
    uint64_t k0, k1;
    get_random_key(&k0, &k1);
    cb_hasher_init(hasher, k0, k1);
}

#define CB_HASHER_O(T, suffix)                                   \
    void cb_hasher_write_##suffix(cb_hasher* hasher, T val)      \
    {                                                            \
        char buf[sizeof(T)];                                     \
        memcpy(&buf, &val, sizeof(T));                           \
        cb_hasher_write(hasher, &buf, sizeof(T));                \
    }                                                            \
                                                                 \
    uint64_t cb_hasher_hash_##suffix(cb_hasher* hasher, T val)   \
    {                                                            \
        cb_hasher_write_##suffix(hasher, val);                   \
        return cb_hasher_finish(hasher);                         \
    }                                                        
CB_HASHER_OVERLOADS
#undef CB_HASHER_O

void cb_hasher_write_str(cb_hasher* hasher, const char* str)
{
    const size_t len = strlen(str);
    cb_hasher_write(hasher, str, len);
}

uint64_t cb_hasher_finish(cb_hasher* hasher)
{
    uint64_t b = (((uint64_t) hasher->len & 0xff) << 56) | hasher->tail;

    hasher->v3 ^= b;
    sip24_compress(hasher);
    hasher->v0 ^= b;

    hasher->v2 ^= 0xff;
    sip24_finalize(hasher);

    return hasher->v0 ^ hasher->v1 ^ hasher->v2 ^ hasher->v3;
}

void cb_hasher_reset(cb_hasher* hasher)
{
    hasher->len = 0;
    hasher->tail = hasher->n_tail = 0;

    hasher->v0 = hasher->k0 ^ 0x736f6d6570736575;
    hasher->v1 = hasher->k1 ^ 0x646f72616e646f6d;
    hasher->v2 = hasher->k0 ^ 0x6c7967656e657261;
    hasher->v3 = hasher->k1 ^ 0x7465646279746573;
}
