#pragma once

#include "cebus/cebus_bool.h"

#include <stdint.h>

uint16_t cb_atomic_fetch_add_u16(volatile uint16_t* dst, uint16_t val);

cebus_bool cb_atomic_compare_exchange_strong_i64(volatile int64_t* dst, int64_t expected, int64_t desired);
cebus_bool cb_atomic_compare_exchange_strong_u64(volatile uint64_t* dst, uint64_t expected, uint64_t desired);
