#pragma once

#include "cebus/dispatch/dispatch_result.h"
#include "cebus/dispatch/message_dispatch.h"
#include "cebus/dispatch/message_handler_invoker.h"

#include "cebus/transport_message.h"

typedef struct cb_message_dispatcher
{
    struct cb_message_dispatcher_impl* impl;
} cb_message_dispatcher;

/// Initialize a new `cb_message_dispatcher`
cb_message_dispatcher* cb_message_dispatcher_init(cb_message_dispatcher* dispatcher);

/// Start the `cb_message_dispatcher` dispatcher
void cb_message_dispatcher_start(cb_message_dispatcher* dispatcher);

/// Stop the `cb_message_dispatcher` dispatcher
void cb_message_dispatcher_stop(cb_message_dispatcher* dispatcher);

void cb_message_dispatcher_register(cb_message_dispatcher* dispatcher, const cb_message_type_id* message_type_id, const cb_message_handler_invoker* invoker);

void cb_message_dispatcher_dispatch(cb_message_dispatcher* dispatcher, cb_message_dispatch* dispatch);

/// Free the memory allocated by the `cb_message_dispatcher` dispatcher
void cb_message_dispatcher_free(cb_message_dispatcher* dispatcher);
