#include "cebus/uuid.h"

#include "cebus/atomic.h"

#include "cebus/utils/random.h"
#include "cebus/utils/time.h"

#include <assert.h>
#include <string.h>

#include <stdio.h>

#define MAX(a, b) ((a) > (b) ? a : b)

cebus_bool cb_time_uuid_gen_init_random(cb_time_uuid_gen* gen)
{
    cebus_bool rc;
    uint8_t node_id[6];
    uint8_t clock_id[2];

    // A UUID-timestamp is a 60 bits 100-nanoseconds interval since 15 october 1582
    const cb_date_time epoch = cb_date_time_from_ymd(1582, 10, 15);

    // The node id should preferrably be a MAC address but generate a random node id for now.
    if ((rc = cb_get_random_bytes(node_id, sizeof(node_id))) == cebus_false)
        return cebus_false;

    // Generate a random clock id
    if ((rc = cb_get_random_bytes(clock_id, sizeof(clock_id))) == cebus_false)
        return cebus_false;

    memcpy(gen->node_id, node_id, sizeof(node_id));
    memcpy(&gen->clock_id, clock_id, sizeof(clock_id));

    gen->epoch_ticks = cb_date_time_ticks(epoch);
    return cebus_true;
}

static uint64_t cb_time_uuid_gen_utc_now_ticks()
{
    const cb_date_time utc_now = cb_date_time_utc_now();
    return cb_date_time_ticks(utc_now);
}

static void cb_time_uuid_gen_ticks(cb_time_uuid_gen* gen, uint64_t ticks, cb_uuid_t* uuid)
{
    const uint64_t rticks = ticks - gen->epoch_ticks;
    uint8_t* bits = uuid->bits;

    // Low timestamp
    bits[0] = (uint8_t) (rticks >> 24);
    bits[1] = (uint8_t) (rticks >> 16);
    bits[2] = (uint8_t) (rticks >> 8);
    bits[3] = (uint8_t) rticks;

    // Mid timestamp
    bits[4] = (uint8_t) (rticks >> 40);
    bits[5] = (uint8_t) (rticks >> 32);

    // High timestamp
    bits[6] = ((uint8_t) (rticks >> 56)) & 0x0F;
    bits[7] = (uint8_t) (rticks >> 48);
    
    bits[8] = (uint8_t) (gen->clock_id >> 8);
    bits[9] = (uint8_t) gen->clock_id;

    memcpy(bits + 10, gen->node_id, sizeof(gen->node_id));

    bits[6] |= 0x10; // set version to 1 (time based uuid)
    bits[8] &= 0x3F; // clear variant
    bits[8] |= 0x80; // set to IETF variant
}

void cb_uuid_generate_time(cb_time_uuid_gen* gen, cb_uuid_t* uuid)
{
    cb_atomic_fetch_add_u16(&gen->clock_id, 1);
    cb_time_uuid_gen_ticks(gen, cb_time_uuid_gen_utc_now_ticks(), uuid);
}

void cb_uuid_print(const cb_uuid_t* uuid, char* buf, size_t size)
{
    static const size_t offsets[] = { 0, 2, 4, 6, 9, 11, 14, 16, 19, 21, 24, 26, 28, 30, 32, 34 };

    static const size_t dash_offsets[] = { 8, 13, 18, 23 };
    static const size_t dashes = sizeof(dash_offsets) / sizeof(*dash_offsets);

    static const char* hex_chars = "0123456789abcdef";

    size_t i;

#define TRY_WRITE(out, index, c)      \
    do {                              \
        if (index >= size)            \
            return;                   \
        buf[index] = c;               \
    } while (0)

    // Write the bits as hexadecimal characters
    for (i = 0; i < CB_UUID_BITS; ++i)
    {
        const uint8_t b = uuid->bits[i];

        TRY_WRITE(buf, offsets[i], hex_chars[b >> 4]);
        TRY_WRITE(buf, offsets[i] + 1, hex_chars[b & 0x0F]); 
    }

    // Write the dashes '-'
    for (i = 0; i < dashes; ++i)
    {
        const size_t dash_offset = dash_offsets[i];
        TRY_WRITE(buf, dash_offset, '-');
    }

    // NUL-terminate the string
    TRY_WRITE(buf, 36, 0);
}
