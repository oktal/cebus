#include "cebus/atomic.h"

#include <stdatomic.h>

#define CB_DEFINE_ATOMIC_ADD(T, suffix)                    \
    T cb_atomic_fetch_add_##suffix(volatile T* dst, T val) \
    {                                                      \
        return atomic_fetch_add((_Atomic(T) *)dst, val);   \
    }

#define CB_DEFINE_ATOMIC_SUB(T, suffix)                    \
    T cb_atomic_fetch_sub_##suffix(volatile T* dst, T val) \
    {                                                      \
        return atomic_fetch_sub((_Atomic(T) *)dst, val);   \
    }

#define CB_DEFINE_ATOMIC_COMPARE_EXCHANGE(T, suffix)                                               \
    cebus_bool cb_atomic_compare_exchange_strong_##suffix(volatile T* dst, T* expected, T desired) \
    {                                                                                              \
        _Bool result = atomic_compare_exchange_strong((_Atomic(T) *)dst, expected, desired);       \
        return cebus_bool_from_c11(result);                                                        \
    }

#define CB_ATOMIC_TYPE(T, suffix) CB_DEFINE_ATOMIC_ADD(T, suffix)
    CB_ATOMIC_TYPES
#undef CB_ATOMIC_TYPE

#define CB_ATOMIC_TYPE(T, suffix) CB_DEFINE_ATOMIC_SUB(T, suffix)
    CB_ATOMIC_TYPES
#undef CB_ATOMIC_TYPE

#define CB_ATOMIC_TYPE(T, suffix) CB_DEFINE_ATOMIC_COMPARE_EXCHANGE(T, suffix)
    CB_ATOMIC_TYPES
#undef CB_ATOMIC_TYPE

#undef CB_DEFINE_ATOMIC_ADD
#undef CB_DEFINE_ATOMIC_SUB
#undef CB_DEFINE_ATOMIC_COMPARE_EXCHANGE
