#include "cebus/atomic.h"

#include <stdatomic.h>

uint16_t cb_atomic_fetch_add_u16(volatile uint16_t* dst, uint16_t val)
{
    return atomic_fetch_add((_Atomic(uint16_t) *)dst, val);
}

cebus_bool cb_atomic_compare_exchange_strong_i64(volatile int64_t* dst, int64_t expected, int64_t desired)
{
    _Bool result = atomic_compare_exchange_strong((_Atomic(int64_t) *)dst, &expected, desired);
    return cebus_bool_from_c11(result);
}

cebus_bool cb_atomic_compare_exchange_strong_u64(volatile uint64_t* dst, uint64_t expected, uint64_t desired)
{
    _Bool result = atomic_compare_exchange_strong((_Atomic(uint64_t) *)dst, &expected, desired);
    return cebus_bool_from_c11(result);
}
