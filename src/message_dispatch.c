#include "cebus/dispatch/message_dispatch.h"

#include "cebus/alloc.h"
#include "cebus/atomic.h"

#include <stdio.h>

static cb_message_dispatch* cb_message_dispatch_init(cb_message_dispatch* dispatch, const cb_transport_message* transport_message, cb_message_dispatch_continuation continuation, void *user)
{
    dispatch->transport_message = transport_message;
    dispatch->continuation = continuation;
    dispatch->user = user;
    dispatch->remaining_handlers_count = 0;

    return dispatch;
}

cb_message_dispatch* cb_message_dispatch_new(const cb_transport_message* transport_message, cb_message_dispatch_continuation continuation, void* user)
{
    return cb_message_dispatch_init(cb_new(cb_message_dispatch, 1), transport_message, continuation, user);
}

void cb_message_dispatch_set_count(cb_message_dispatch* dispatch, size_t count)
{
    dispatch->remaining_handlers_count = (uint64_t) count;
}

void cb_message_dispatch_set_ignored(cb_message_dispatch* dispatch)
{
    if (dispatch->continuation != NULL)
        dispatch->continuation(dispatch, dispatch->user);
}

void cb_message_dispatch_set_handled(cb_message_dispatch* dispatch)
{
    uint64_t remaining = cb_atomic_fetch_sub_u64(&dispatch->remaining_handlers_count, 1) - 1;
    if (remaining == 0)
    {
        if (dispatch->continuation != NULL)
            dispatch->continuation(dispatch, dispatch->user);
    }
}
