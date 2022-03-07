#include "cebus/threading.h"

#include "cebus/alloc.h"
#include "protobuf-c/protobuf-c.h"

#include <stdio.h>
#include <stdlib.h>

typedef struct cb_future_get_continuation_state
{
    cb_mutex_t mutex;
    cb_cond_t cond;

    void* result;
} cb_future_get_continuation_state;

typedef struct cb_future_inner
{
    cb_mutex_t mutex;

    void* data;

    cb_future_state state;

    cb_future_continuation continuation;
    cb_future_destructor destructor;

    void* user;
} cb_future_inner;

static void cb_future_get_continuation_state_init(cb_future_get_continuation_state* state)
{
    cb_mutex_init(&state->mutex);
    cb_cond_init(&state->cond);
}

static void cb_future_get_continuation(void *user, void *result)
{
    cb_future_get_continuation_state* state = (cb_future_get_continuation_state *) user;
    cb_mutex_lock(&state->mutex);
    state->result = result;
    cb_cond_signal(&state->cond);
    cb_mutex_unlock(&state->mutex);
}

void cb_thread_init(cb_thread* thread)
{
    thread->data = NULL;
}

cebus_bool cb_thread_joinable(cb_thread* thread)
{
    return cebus_bool_from_int(thread->data != NULL);
}

void cb_thread_destroy(cb_thread* thread)
{
    free(thread->data);
    thread->data = NULL;
}

void cb_future_init(cb_future* future, cb_future_destructor destructor)
{
    cb_future_inner* inner = cb_new(cb_future_inner, 1);
    inner->data = NULL;
    inner->user = NULL;
    cb_mutex_init(&inner->mutex);
    inner->state = cb_future_pending;
    inner->continuation = NULL;
    inner->destructor = destructor;

    future->inner = inner;
}

cb_future_state cb_future_poll(cb_future* future, void** data_out)
{
    cb_future_state state;
    void* data;
    cb_future_inner* inner = (cb_future_inner *) future->inner;

    cb_mutex_lock(&inner->mutex);
    data = inner->data;
    state = inner->state;
    cb_mutex_unlock(&inner->mutex);

    if (state == cb_future_ready)
    {
        *data_out = data;
        return cb_future_ready;
    }

    *data_out = NULL;
    return cb_future_pending;
}

void cb_future_then(cb_future* future, cb_future_continuation continuation, void* user)
{
    cb_future_inner* inner = (cb_future_inner *) future->inner;

    cb_mutex_lock(&inner->mutex);
    if (inner->state == cb_future_ready)
    {
        // Do not hold mutex while calling user callaback
        cb_mutex_unlock(&inner->mutex);
        continuation(user, inner->data);
    }
    else
    {
        inner->user = user;
        inner->continuation = continuation;
        cb_mutex_unlock(&inner->mutex);
    }
}

void cb_future_set(cb_future* future, void* data)
{
    cb_future_inner* inner = (cb_future_inner *) future->inner;

    cb_mutex_lock(&inner->mutex);
    if (inner->state == cb_future_pending)
    {
        inner->data = data;
        inner->state = cb_future_ready;
        if (inner->continuation != NULL)
        {
            // Do not hold mutex while calling user callaback
            cb_mutex_unlock(&inner->mutex);
            inner->continuation(inner->user, inner->data);
        }
        else
        {
            cb_mutex_unlock(&inner->mutex);
        }
    }
    else
    {
        cb_mutex_unlock(&inner->mutex);
    }
}

void* cb_future_get(cb_future* future)
{
    cb_future_get_continuation_state state;
    cb_future_get_continuation_state_init(&state);

    cb_future_then(future, cb_future_get_continuation, &state);
    cb_cond_wait(&state.cond, &state.mutex);
    return state.result;
}

void cb_future_destroy(cb_future *future)
{
    cb_future_inner* inner = (cb_future_inner *) future->inner;

    if (inner->destructor != NULL)
        inner->destructor(inner->user, inner->data);

    free(future->inner);
    future->inner = NULL;
}
