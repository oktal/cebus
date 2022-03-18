#include "cebus/dispatch/message_handler_invoker.h"

#include "cebus/alloc.h"

#include <string.h>

cb_message_handler_invoker* cb_message_handler_invoker_init(cb_message_handler_invoker* invoker, const char* dispatch_queue, cb_message_handler_invoker_func func, void* user)
{
    const char* queue = dispatch_queue == NULL ? CB_DISPATCH_QUEUE_DEFAULT : dispatch_queue;

    strncpy(invoker->dispatch_queue, queue, CEBUS_STR_MAX);
    invoker->func = func;
    invoker->user = user;
    return invoker;
}

cb_message_handler_invoker* cb_message_handler_invoker_copy(cb_message_handler_invoker* dst, const cb_message_handler_invoker* src)
{
    strncpy(dst->dispatch_queue, src->dispatch_queue, CEBUS_STR_MAX);
    dst->func = src->func;
    dst->user = src->user;
    return dst;
}
