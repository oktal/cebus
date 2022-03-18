#pragma once

#include "cebus/config.h"
#include "cebus/transport_message.h"

#define CB_DISPATCH_QUEUE_DEFAULT "__CB_DISPATCH_QUEUE_DEFAULT"

typedef void (*cb_message_handler_invoker_func)(const cb_transport_message* transport_message, void* user);

typedef struct cb_message_handler_invoker
{
    char dispatch_queue[CEBUS_STR_MAX];

    void* user;

    cb_message_handler_invoker_func func;
} cb_message_handler_invoker;

cb_message_handler_invoker* cb_message_handler_invoker_init(cb_message_handler_invoker* invoker, const char* dispatch_queue, cb_message_handler_invoker_func func, void* user);
cb_message_handler_invoker* cb_message_handler_invoker_copy(cb_message_handler_invoker* dst, const cb_message_handler_invoker* src);
