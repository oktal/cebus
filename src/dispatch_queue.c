#include "cebus/dispatch/dispatch_queue.h"

#include "cebus/collection/array.h"
#include "cebus/alloc.h"
#include "cebus/atomic.h"
#include "cebus/threading.h"

#include <string.h>

typedef struct cb_dispatch_queue_entry
{
    cb_message_dispatch* dispatch;

    void* user;

    cb_message_handler_invoker_func invoker;

    struct cb_dispatch_queue_entry* next;
} cb_dispatch_queue_entry;

#define CB_DISPATCH_QUEUE_INIT_SIZE 512

typedef struct cb_dispatch_queue_impl
{
    char name[CEBUS_STR_MAX];

    cb_dispatch_queue_entry* head;

    cb_dispatch_queue_entry* tail;

    cb_mutex_t queue_mutex;

    cb_cond_t queue_cond;

    cb_thread thread;

    int64_t running;
} cb_dispatch_queue_impl;

static void* cb_dispatch_queue_impl_thread(void* data)
{
    cb_dispatch_queue_impl* impl = (cb_dispatch_queue_impl *) data;
    cebus_bool running = cebus_true;
    while (running == cebus_true)
    {
        cb_dispatch_queue_entry* entry;

        cb_cond_wait(&impl->queue_cond, &impl->queue_mutex);
        entry = impl->head;
        impl->head = impl->tail = NULL;

        cb_mutex_unlock(&impl->queue_mutex);

        while (entry != NULL)
        {
            cb_dispatch_queue_entry* tmp = entry;
            if (entry->dispatch == NULL)
            {
                running = cebus_false;
                break;
            }

            entry->invoker(entry->dispatch->transport_message, entry->user);
            cb_message_dispatch_set_handled(entry->dispatch);

            entry = entry->next;
            free(tmp);
        }
    }

    return NULL;
}

static cb_dispatch_queue_entry* cb_dispatch_queue_entry_init(cb_dispatch_queue_entry* entry, cb_message_dispatch* dispatch, const cb_message_handler_invoker* invoker)
{
    entry->dispatch = dispatch;
    entry->user = invoker->user;
    entry->invoker = invoker->func;
    entry->next = NULL;

    return entry;
}

static void cb_dispatch_queue_impl_enqueue_entry(cb_dispatch_queue_impl* impl, cb_dispatch_queue_entry* entry)
{
    cb_mutex_lock(&impl->queue_mutex);

    if (impl->head == NULL)
    {
        impl->head = impl->tail = entry;
    }
    else
    {
        impl->tail->next = entry;
        impl->tail = entry;
    }

    cb_cond_signal(&impl->queue_cond);
    cb_mutex_unlock(&impl->queue_mutex);
}

static cb_dispatch_queue_impl* cb_dispatch_queue_impl_init(cb_dispatch_queue_impl* impl, const char* name)
{
    strncpy(impl->name, name, CEBUS_STR_MAX);
    impl->head = impl->tail = NULL;
    cb_mutex_init(&impl->queue_mutex);
    cb_cond_init(&impl->queue_cond);
    cb_thread_init(&impl->thread);

    impl->running = 0;

    return impl;
}

static void cb_dispatch_queue_impl_start(cb_dispatch_queue_impl* impl)
{
    int64_t expected = 0;
    if (cb_atomic_compare_exchange_strong_i64(&impl->running, &expected, 1) == cebus_false)
        return;

    cb_thread_spawn(&impl->thread, cb_dispatch_queue_impl_thread, impl);
}

static void cb_dispatch_queue_impl_stop(cb_dispatch_queue_impl* impl)
{
    int64_t expected = 1;
    if (cb_atomic_compare_exchange_strong_i64(&impl->running, &expected, 0) == cebus_false)
        return;

    cb_dispatch_queue_impl_enqueue_entry(impl, cb_dispatch_queue_entry_init(cb_new(cb_dispatch_queue_entry, 1), NULL, NULL));
    cb_thread_join(&impl->thread);
}

static void cb_dispatch_queue_impl_enqueue(cb_dispatch_queue_impl* impl, cb_message_dispatch* dispatch, const cb_message_handler_invoker* invoker)
{
    cb_dispatch_queue_impl_enqueue_entry(impl, cb_dispatch_queue_entry_init(cb_new(cb_dispatch_queue_entry, 1), dispatch, invoker));
}

static void cb_dispatch_queue_impl_free(cb_dispatch_queue_impl* impl)
{
    cb_dispatch_queue_impl_stop(impl);
    cb_mutex_destroy(&impl->queue_mutex);
    cb_cond_destroy(&impl->queue_cond);
    cb_thread_destroy(&impl->thread);
}

cb_dispatch_queue* cb_dispatch_queue_init(cb_dispatch_queue* queue, const char* name)
{
    queue->impl = cb_dispatch_queue_impl_init(cb_new(cb_dispatch_queue_impl, 1), name);
    return queue;
}

void cb_dispatch_queue_start(cb_dispatch_queue* queue)
{
    cb_dispatch_queue_impl_start(queue->impl);
}

void cb_dispatch_queue_stop(cb_dispatch_queue* queue)
{
    cb_dispatch_queue_impl_stop(queue->impl);
}

void cb_dispatch_queue_enqueue(cb_dispatch_queue* queue, cb_message_dispatch* dispatch, const cb_message_handler_invoker* invoker)
{
    cb_dispatch_queue_impl_enqueue(queue->impl, dispatch, invoker);
}

void cb_dispatch_queue_free(cb_dispatch_queue* queue)
{
    cb_dispatch_queue_impl_free(queue->impl);
    free(queue->impl);
}
