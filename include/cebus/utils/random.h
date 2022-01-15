#pragma once

#include "cebus/cebus_bool.h"

#include <stdint.h>
#include <stddef.h>

/// Fill the buffer pointed by `buf` with `len` random bytes.
/// Return `cebus_true` on success.
cebus_bool cb_get_random_bytes(uint8_t* buf, size_t len);
