#pragma once

#include "cebus/dispatch/message_dispatch.h"
#include "cebus/dispatch/message_handler_invoker.h"

typedef struct cb_dispatch_queue
{
    struct cb_dispatch_queue_impl *impl;
} cb_dispatch_queue;

/// Initialize a new `name`d `cb_dispatch_queue` queue
cb_dispatch_queue* cb_dispatch_queue_init(cb_dispatch_queue* queue, const char* name);

/// Start the `cb_dispatch_queue` queue
void cb_dispatch_queue_start(cb_dispatch_queue* queue);

/// Stop the `cb_dispatch_queue` queue
void cb_dispatch_queue_stop(cb_dispatch_queue* queue);

void cb_dispatch_queue_enqueue(cb_dispatch_queue* queue, cb_message_dispatch* dispatch, const cb_message_handler_invoker* invoker);

/// Free the memory allocated by the `cb_dispatch_queue` queue
void cb_dispatch_queue_free(cb_dispatch_queue* queue);
