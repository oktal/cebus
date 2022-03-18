#pragma once

#include "cebus/transport_message.h"

#include <stddef.h>
#include <stdint.h>

struct cb_message_dispatch;
typedef void (*cb_message_dispatch_continuation)(struct cb_message_dispatch* dispatch, void* user);

typedef struct cb_message_dispatch
{
    const cb_transport_message* transport_message;

    cb_message_dispatch_continuation continuation;

    void* user;

    uint64_t remaining_handlers_count;
} cb_message_dispatch;

/// Create a new `cb_message_dispatch`
cb_message_dispatch* cb_message_dispatch_new(const cb_transport_message* transport_message, cb_message_dispatch_continuation continuation, void* user);

/// Mark the given `cb_message_dispatch` dispatch as ignored
void cb_message_dispatch_set_ignored(cb_message_dispatch* dispatch);

/// Set the total number of handlers to `count` for given `cb_message_dispatch` dispatch
void cb_message_dispatch_set_count(cb_message_dispatch* dispatch, size_t count);

/// Mark the given `cb_message_dispatch` dispatch as handled
void cb_message_dispatch_set_handled(cb_message_dispatch* dispatch);
