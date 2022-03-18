#pragma once

#include "cebus/cebus_bool.h"

#include <stdint.h>

#define CB_ATOMIC_TYPES           \
    CB_ATOMIC_TYPE(int8_t,   i8)  \
    CB_ATOMIC_TYPE(int16_t,  i16) \
    CB_ATOMIC_TYPE(int32_t,  i32) \
    CB_ATOMIC_TYPE(int64_t,  i64) \
    CB_ATOMIC_TYPE(uint8_t,  u8)  \
    CB_ATOMIC_TYPE(uint16_t, u16) \
    CB_ATOMIC_TYPE(uint32_t, u32) \
    CB_ATOMIC_TYPE(uint64_t, u64) \

#define CB_DECLARE_ATOMIC_ADD(T, suffix) \
    T cb_atomic_fetch_add_##suffix(volatile T* dst, T val);

#define CB_DECLARE_ATOMIC_SUB(T, suffix) \
    T cb_atomic_fetch_sub_##suffix(volatile T* dst, T val);

#define CB_DECLARE_ATOMIC_COMPARE_EXCHANGE(T, suffix) \
    cebus_bool cb_atomic_compare_exchange_strong_##suffix(volatile T* dst, T* expected, T desired);

#define CB_ATOMIC_TYPE(T, suffix) CB_DECLARE_ATOMIC_ADD(T, suffix)
  CB_ATOMIC_TYPES
#undef CB_ATOMIC_TYPE

#define CB_ATOMIC_TYPE(T, suffix) CB_DECLARE_ATOMIC_SUB(T, suffix)
  CB_ATOMIC_TYPES
#undef CB_ATOMIC_TYPE

#define CB_ATOMIC_TYPE(T, suffix) CB_DECLARE_ATOMIC_COMPARE_EXCHANGE(T, suffix)
  CB_ATOMIC_TYPES
#undef CB_ATOMIC_TYPE

#undef CB_DECLARE_ATOMIC_ADD
#undef CB_DECLARE_ATOMIC_SUB
#undef CB_DECLARE_ATOMIC_COMPARE_EXCHANGE
