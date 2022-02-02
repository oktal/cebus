#include "cebus/threading.h"

#include <stdlib.h>

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

void cb_future_init(cb_future* future)
{
    future->data = NULL;
    cb_mutex_init(&future->mutex);
    future->state = cb_future_pending;
    future->awaiter = NULL;
}

cb_future_state cb_future_poll(cb_future* future, void** data_out)
{
    cb_future_state state;
    void* data;

    cb_mutex_lock(&future->mutex);
    data = future->data;
    state = future->state;
    cb_mutex_unlock(&future->mutex);

    if (state == cb_future_ready)
    {
        *data_out = data;
        return cb_future_ready;
    }

    *data_out = NULL;
    return cb_future_pending;
}

void cb_future_await(cb_future* future, cb_future_awaiter awaiter, void* user)
{
    cb_mutex_lock(&future->mutex);
    if (future->state == cb_future_ready)
    {
        void* user = future->user;
        void* data = future->data;
        // Do not hold mutex while calling user callaback
        cb_mutex_unlock(&future->mutex);
        awaiter(future->data, user);
    }
    else
    {
        future->awaiter = awaiter;
        future->user = user;
        cb_mutex_unlock(&future->mutex);
    }
}

void cb_future_set(cb_future* future, void* data)
{
    cb_mutex_lock(&future->mutex);
    if (future->state == cb_future_pending)
    {
        future->data = data;
        future->state = cb_future_ready;
        if (future->awaiter != NULL)
        {
            // Do not hold mutex while calling user callaback
            cb_mutex_unlock(&future->mutex);
            future->awaiter(future->data, future->user);
        }
        else
        {
            cb_mutex_unlock(&future->mutex);
        }
    }
    else
    {
        cb_mutex_unlock(&future->mutex);
    }
}
