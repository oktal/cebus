#pragma once

#include "cebus/collection/hash_map.h"
#include "cebus/message_type_id.h"
#include "cebus/transport_message.h"

#include <stddef.h>

typedef void (*cb_message_callback)(const cb_transport_message* transport_message, void *user);

typedef struct cb_message_dispatcher
{
    cb_hash_map* message_callbacks;
} cb_message_dispatcher;

/// Initialize a new `cb_message_dispatcher`
cb_message_dispatcher* cb_message_dispatcher_init(cb_message_dispatcher* dispatcher);

/// Add a new `cb_message_callback` to invoke when receiving a message of type `message_type_id`
void cb_message_dispatcher_add(cb_message_dispatcher* dispatcher, const cb_message_type_id* message_type_id, cb_message_callback callback, void* user);

/// Dispatch the `cb_transport_message` message to the corresponding handlers.
/// Return the number of calls that were made
size_t cb_message_dispatcher_dispatch(cb_message_dispatcher* dispatcher, const cb_message_type_id* message_type_id, const cb_transport_message* message);

/// Free the memory allocated by the `cb_message_dispatcher` dispatcher
void cb_message_dispatcher_free(cb_message_dispatcher* dispatcher);
