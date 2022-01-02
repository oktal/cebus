#include "cebus/threading.h"

#include "cebus/alloc.h"

#include <stdlib.h>

void cb_thread_init(cb_thread* thread)
{
    thread->is_running = cebus_false;
}

cebus_bool cb_thread_spawn(cb_thread* thread, cb_thread_start start, void* arg)
{
    pthread_attr_t attr;
    pthread_t* handle = &thread->handle;
    int rc;

    pthread_attr_init(&attr);
    rc = pthread_create(handle, &attr, start, arg);
    if (rc < 0)
    {
        pthread_attr_destroy(&attr);
        return cebus_false;
    }

    pthread_attr_destroy(&attr);
    return cebus_true;
}

cebus_bool cb_thread_joinable(cb_thread* thread)
{
    return thread->is_running;
}

void* cb_thread_join(cb_thread* thread)
{
    void* result;
    pthread_join(thread->handle, &result);
    return result;
}

void cb_thread_free(cb_thread* thread)
{
    free(thread);
}
