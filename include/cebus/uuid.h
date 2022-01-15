#pragma once

#include "cebus/cebus_bool.h"

#include <stdint.h>
#include <stddef.h>

#define CB_UUID_BITS 16

typedef struct cb_uuid_t
{
    uint8_t bits[CB_UUID_BITS];
} cb_uuid_t;

typedef struct cb_time_uuid_gen
{
    uint8_t node_id[6];
    uint16_t clock_id;

    uint64_t epoch_ticks;
} cb_time_uuid_gen;

#define CB_TIME_UUID_GEN_INIT { { 0 }, 0, 0 }

/// Initialize the time-based uuid generator with random bytes
cebus_bool cb_time_uuid_gen_init_random(cb_time_uuid_gen* gen);

/// Generate a time-based uuid based on https://www.famkruithof.net/guid-uuid-timebased.html
void cb_uuid_generate_time(cb_time_uuid_gen* gen, cb_uuid_t* uuid);

/// Print a string representation of the given `uuid` to a buffer pointed by `buf`
void cb_uuid_print(const cb_uuid_t* uuid, char* buf, size_t size);
