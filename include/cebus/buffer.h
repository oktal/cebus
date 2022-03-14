#pragma once

#include <stddef.h>

/// A `cb_buffer` represents contiguous bytes in memory
typedef struct cb_buffer
{
    /// The data that this buffer holds
    void* data;

    /// The size of the data this buffer holds
    size_t n_data;
} cb_buffer;

/// Initialize a new `cb_buffer` buffer
/// Return `buffer`
cb_buffer* cb_buffer_init(cb_buffer* buffer);

/// Copy a `cb_buffer` from `src` to `dst`.
/// Return `dst`
cb_buffer* cb_buffer_copy(cb_buffer* dst, const cb_buffer* src);

/// Move a `cb_buffer` from `src` to `dst`. A move operation will invalide `src`
/// Return `dst`
cb_buffer* cb_buffer_move(cb_buffer* dst, cb_buffer* src);

/// Free the memory owned by this `buffer`
void cb_buffer_free(cb_buffer* buffer);
