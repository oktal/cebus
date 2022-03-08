#include "cebus/dispatch/message_dispatcher.h"

#include "cebus/alloc.h"
#include "cebus/collection/array.h"
#include "cebus/message_type_id.h"

typedef struct cb_message_dispatch_callback_entry
{
    cb_message_callback callback;

    void *user;
} cb_message_dispatch_callback_entry;

#define CB_MESSAGE_DISPATCH_CALLBACKS_INIT_CAPACITY 8

typedef struct cb_message_dispatch_entry
{
    cb_message_type_id message_type_id;

    cb_array callbacks;

} cb_message_dispatch_entry;

static cb_message_dispatch_entry* cb_message_dispatch_entry_init(cb_message_dispatch_entry* entry)
{
    cb_array_init_with_capacity(&entry->callbacks, CB_MESSAGE_DISPATCH_CALLBACKS_INIT_CAPACITY, sizeof(cb_message_dispatch_callback_entry));
    return entry;
}

static size_t cb_message_dispatch_entry_dispatch(cb_message_dispatch_entry* entry, const cb_transport_message* message)
{
    size_t count = 0;
    cb_array_iterator iter = cb_array_iter(&entry->callbacks);
    while (cb_array_iter_has_next(CB_ARRAY_ITER(iter)) == cebus_true)
    {
        cb_message_dispatch_callback_entry* callback_entry = (cb_message_dispatch_callback_entry *) cb_array_iter_get(iter);
        callback_entry->callback(message, callback_entry->user);
        ++count;
        cb_array_iter_move_next(CB_ARRAY_ITER(iter));
    }

    return count;
}

static void cb_message_dispatch_entry_free(cb_message_dispatch_entry* entry)
{
    cb_array_free(&entry->callbacks, NULL, NULL);
}

static void cb_message_dispatch_entry_hash_free(cb_hash_key_t key, cb_hash_value_t value, void* user)
{
    cb_message_type_id* message_type_id = (cb_message_type_id *) key;
    cb_message_dispatch_entry* entry = (cb_message_dispatch_entry *) value;
}

static cb_message_dispatch_entry* cb_message_dispatcher_get_or_create_entry(cb_message_dispatcher* dispatcher, const cb_message_type_id* message_type_id)
{
    cb_hash_value_t entry = cb_hash_get(dispatcher->message_callbacks, message_type_id);
    if (entry == NULL)
    {
        cb_message_dispatch_entry* dispatch_entry = cb_message_dispatch_entry_init(cb_new(cb_message_dispatch_entry, 1));
        entry = cb_hash_insert(dispatcher->message_callbacks, cb_message_type_id_clone(message_type_id), dispatch_entry);
    }

    return (cb_message_dispatch_entry *) entry;
}

cb_message_dispatcher* cb_message_dispatcher_init(cb_message_dispatcher *dispatcher)
{
    dispatcher->message_callbacks = cb_hash_map_new(cb_message_type_id_hash, cb_message_type_id_hash_eq);
    return dispatcher;
}

void cb_message_dispatcher_add(cb_message_dispatcher* dispatcher, const cb_message_type_id* message_type_id, cb_message_callback callback, void* user)
{
    cb_message_dispatch_entry* entry = cb_message_dispatcher_get_or_create_entry(dispatcher, message_type_id);
    cb_message_dispatch_callback_entry* callback_entry = (cb_message_dispatch_callback_entry *) cb_array_push(&entry->callbacks);
    callback_entry->callback = callback;
    callback_entry->user = user;
}

size_t cb_message_dispatcher_dispatch(cb_message_dispatcher *dispatcher, const cb_message_type_id *message_type_id, const cb_transport_message *message)
{
    size_t count = 0;
    cb_hash_value_t entry = cb_hash_get(dispatcher->message_callbacks, message_type_id);
    if (entry != NULL)
        count = cb_message_dispatch_entry_dispatch((cb_message_dispatch_entry *) entry, message);

    return count;
}

void cb_message_dispatcher_free(cb_message_dispatcher *dispatcher)
{
    cb_hash_map_free(dispatcher->message_callbacks, cb_message_dispatch_entry_hash_free, NULL);
    free(dispatcher->message_callbacks);
}
