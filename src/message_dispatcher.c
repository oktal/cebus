#include "cebus/dispatch/message_dispatcher.h"

#include "cebus/alloc.h"
#include "cebus/collection/array.h"
#include "cebus/collection/hash_map.h"
#include "cebus/config.h"
#include "cebus/dispatch/dispatch_queue.h"
#include "cebus/log.h"
#include "cebus/message_type_id.h"

#include <string.h>

typedef struct cb_dispatch_queue_entry
{
    // The name of the dispatch queue
    char* name;

    // The dispatch queue
    cb_dispatch_queue dispatch_queue;
} cb_dispatch_queue_entry;

typedef struct cb_message_dispatcher_invoker_entry
{
    cb_array invokers;
} cb_message_handler_invoker_entry;

typedef struct cb_message_dispatcher_impl
{
    // The list of dispatch queues
    cb_array dispatch_queues;

    // Index of message invokers by mesage type id
    cb_hash_map* message_invokers;
} cb_message_dispatcher_impl;

static cb_dispatch_queue_entry* cb_dispatch_queue_entry_init(cb_dispatch_queue_entry* entry, const char* name)
{
    entry->name = cb_strdup(name);
    cb_dispatch_queue_init(&entry->dispatch_queue, name);
    return entry;
}

static cb_message_handler_invoker_entry* cb_message_handler_invoker_entry_init(cb_message_handler_invoker_entry* entry)
{
    cb_array_init_with_capacity(&entry->invokers, 8, sizeof(cb_message_handler_invoker));
    return entry;
}

static void cb_message_handler_invoker_entry_free(cb_message_handler_invoker_entry* entry)
{
    cb_array_free(&entry->invokers, NULL, NULL);
}

static cb_dispatch_queue_entry* cb_message_dispatch_impl_get_dispatch_queue(cb_message_dispatcher_impl* impl, const char* name)
{
    cb_array_iterator_mut iter = cb_array_iter_mut(&impl->dispatch_queues);
    while (cb_array_iter_has_next(CB_ARRAY_ITER(iter)) == cebus_true)
    {
        cb_dispatch_queue_entry* entry = cb_array_iter_get_mut(iter);
        if (strcmp(entry->name, name) == 0)
            return entry;

        cb_array_iter_move_next(CB_ARRAY_ITER(iter));
    }

    return NULL;
}

static cb_message_dispatcher_impl* cb_message_dispatcher_impl_init(cb_message_dispatcher_impl* impl)
{
    cb_array_init_with_capacity(&impl->dispatch_queues, 8, sizeof(cb_dispatch_queue_entry));
    cb_dispatch_queue_entry_init((cb_dispatch_queue_entry *) cb_array_push(&impl->dispatch_queues), CB_DISPATCH_QUEUE_DEFAULT);
    impl->message_invokers = cb_hash_map_new(cb_message_type_id_hash, cb_message_type_id_hash_eq);
    return impl;
}

static void cb_message_dispatcher_impl_start(cb_message_dispatcher_impl *impl)
{
    cb_array_iterator_mut iter = cb_array_iter_mut(&impl->dispatch_queues);
    while (cb_array_iter_has_next(CB_ARRAY_ITER(iter)) == cebus_true)
    {
        cb_dispatch_queue_entry* entry = cb_array_iter_get_mut(iter);
        cb_dispatch_queue_start(&entry->dispatch_queue);
        cb_array_iter_move_next(CB_ARRAY_ITER(iter));
    }
}

static void cb_message_dispatcher_impl_stop(cb_message_dispatcher_impl *impl)
{
    cb_array_iterator_mut iter = cb_array_iter_mut(&impl->dispatch_queues);
    while (cb_array_iter_has_next(CB_ARRAY_ITER(iter)) == cebus_true)
    {
        cb_dispatch_queue_entry* entry = cb_array_iter_get_mut(iter);
        cb_dispatch_queue_stop(&entry->dispatch_queue);
        cb_array_iter_move_next(CB_ARRAY_ITER(iter));
    }
}

static void cb_dispatch_queue_entry_array_free(void* value, void* user)
{
    cb_dispatch_queue_entry *entry = (cb_dispatch_queue_entry *) value;
    free(entry->name);
    cb_dispatch_queue_free(&entry->dispatch_queue);
}

static void cb_message_handler_invoker_entry_hash_free(cb_hash_key_t key, cb_hash_value_t value, void* user)
{
    cb_message_type_id* message_type_id = (cb_message_type_id *) key;
    cb_message_handler_invoker_entry* entry = (cb_message_handler_invoker_entry *) value;

    free(message_type_id);
    cb_message_handler_invoker_entry_free(entry);
}


static void cb_message_dispatcher_impl_free(cb_message_dispatcher_impl* impl)
{
    cb_message_dispatcher_impl_stop(impl);
    cb_array_free(&impl->dispatch_queues, cb_dispatch_queue_entry_array_free, NULL);
    cb_hash_map_free(impl->message_invokers, cb_message_handler_invoker_entry_hash_free, NULL);
    free(impl->message_invokers);
}

static void cb_message_dispatcher_impl_dispatch_one(cb_message_dispatcher_impl* impl, cb_message_dispatch* dispatch, const cb_message_handler_invoker* invoker)
{
    cb_dispatch_queue_entry* dispatch_queue_entry = cb_message_dispatch_impl_get_dispatch_queue(impl, invoker->dispatch_queue);
    if (dispatch_queue_entry == NULL)
    {
        CB_LOG_DBG(CB_LOG_LEVEL_WARN, "unknown dispatch queue %s when dispatching %s", invoker->dispatch_queue, dispatch->transport_message->message_type_id.value);
        return;
    }

    CB_LOG_DBG(CB_LOG_LEVEL_TRACE, "Dispatching %s on dispatch queue %s",
            dispatch->transport_message->message_type_id.value, dispatch_queue_entry->name);

    cb_dispatch_queue_enqueue(&dispatch_queue_entry->dispatch_queue, dispatch, invoker);
}

static void cb_message_dispatcher_impl_dispatch_all(cb_message_dispatcher_impl* impl, cb_message_dispatch* dispatch, const cb_message_handler_invoker_entry* invokers)
{
    if (invokers == NULL || cb_array_empty(&invokers->invokers) == cebus_true)
    {
        cb_message_dispatch_set_ignored(dispatch);
        return;
    }

    cb_message_dispatch_set_count(dispatch, cb_array_size(&invokers->invokers));

    {
        cb_array_iterator iter = cb_array_iter(&invokers->invokers);
        while (cb_array_iter_has_next(CB_ARRAY_ITER(iter)) == cebus_true)
        {
            cb_message_handler_invoker* invoker = (cb_message_handler_invoker *) cb_array_iter_get(iter);
            cb_message_dispatcher_impl_dispatch_one(impl, dispatch, invoker);
            cb_array_iter_move_next(CB_ARRAY_ITER(iter));
        }
    }
}

static cb_message_handler_invoker_entry* cb_message_dispatcher_impl_get_or_create_invoker_entry(cb_message_dispatcher_impl* impl, const cb_message_type_id* message_type_id)
{
    cb_hash_value_t entry = cb_hash_get(impl->message_invokers, message_type_id);
    if (entry == NULL)
        entry = cb_hash_insert(impl->message_invokers, cb_message_type_id_clone(message_type_id), cb_message_handler_invoker_entry_init(cb_new(cb_message_handler_invoker_entry, 1)));

    return (cb_message_handler_invoker_entry *) entry;
}

static void cb_message_dispatcher_impl_register(cb_message_dispatcher_impl* impl, const cb_message_type_id* message_type_id, const cb_message_handler_invoker* invoker)
{
    cb_message_handler_invoker_entry* entry = cb_message_dispatcher_impl_get_or_create_invoker_entry(impl, message_type_id);
    cb_message_handler_invoker* invoker_entry = (cb_message_handler_invoker *) cb_array_push(&entry->invokers);
    cb_message_handler_invoker_copy(invoker_entry, invoker);
}

cb_message_dispatcher* cb_message_dispatcher_init(cb_message_dispatcher* dispatcher)
{
    dispatcher->impl = cb_message_dispatcher_impl_init(cb_new(cb_message_dispatcher_impl, 1));
    return dispatcher;
}

void cb_message_dispatcher_start(cb_message_dispatcher* dispatcher)
{
    cb_message_dispatcher_impl_start(dispatcher->impl);
}

void cb_message_dispatcher_stop(cb_message_dispatcher* dispatcher)
{
    cb_message_dispatcher_impl_stop(dispatcher->impl);
}

void cb_message_dispatcher_register(cb_message_dispatcher* dispatcher, const cb_message_type_id* message_type_id, const cb_message_handler_invoker* invoker)
{
    cb_message_dispatcher_impl_register(dispatcher->impl, message_type_id, invoker);
}

void cb_message_dispatcher_dispatch(cb_message_dispatcher* dispatcher, cb_message_dispatch* dispatch)
{
    cb_message_dispatcher_impl* impl = dispatcher->impl;
    const cb_message_type_id* message_type_id = &dispatch->transport_message->message_type_id;
    cb_message_handler_invoker_entry* invokers = (cb_message_handler_invoker_entry *) cb_hash_get(impl->message_invokers, message_type_id);

    CB_LOG_DBG(CB_LOG_LEVEL_DEBUG, "Dispatching %s", dispatch->transport_message->message_type_id.value);
    cb_message_dispatcher_impl_dispatch_all(impl, dispatch, invokers);
}

void cb_message_dispatcher_free(cb_message_dispatcher* dispatcher)
{
    cb_message_dispatcher_impl_free(dispatcher->impl);
    free(dispatcher->impl);
}
