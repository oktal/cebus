#include "cebus/bus.h"

#include "cebus/alloc.h"
#include "cebus/atomic.h"
#include "cebus/collection/array.h"
#include "cebus/collection/hash_map.h"
#include "cebus/dispatch/message_dispatch.h"
#include "cebus/log.h"
#include "cebus/message_id.h"
#include "cebus/threading.h"
#include "cebus/uuid.h"

#include "message_execution_completed.pb-c.h"

#include <stdio.h>
#include <string.h>

#define CB_VA_ARGS(...) , ##__VA_ARGS__
#define CB_BUS_VIRT_SET(base, func_name, func) \
    base->func_name##_func = func;
#define CB_BUS_VIRT_CALL(base, func_name, ...) \
    base->func_name##_func(base CB_VA_ARGS(__VA_ARGS__))

typedef struct cb_bus_impl
{
    // The base interface. Must be the first field
    cb_bus base;

    // Map of message sending actions to ongoing tasks
    cb_hash_map* message_id_tasks;

    // Transport layer
    cb_transport* transport;

    // Tell whether the bus is running or not
    uint64_t is_running;

    // Current peer id
    cb_peer_id peer_id;

    // Inbound endpoint
    const char* inbound_endpoint;

    // Current environment
    char environment[CEBUS_STR_MAX];

    // UUID generator
    cb_time_uuid_gen uuid_gen;

    // Message dispatcher
    cb_message_dispatcher message_dispatcher;
} cb_bus_impl;

// An ongoing command
typedef struct cb_command_future
{
    // The original command that has been sent
    cb_command command;

    // The transport message
    cb_transport_message* transport_message;

    // The result of the command
    cb_command_result result;

    cb_future future;
} cb_command_future;

static void cb_command_future_free(cb_command_future *future)
{
    free(future->transport_message);
}

static void cb_command_future_destroy(void *user, void *data)
{
    cb_command_future* command_future = (cb_command_future *) data;
    cb_command_future_free((cb_command_future *) data);
    free(data);
}

static void cb_command_future_init(cb_command_future* command_future)
{
    cb_command_init(&command_future->command);
    command_future->transport_message = NULL;
    cb_future_init(&command_future->future, sizeof(cb_command_result));
}

static void cb_bus_impl_message_id_task_hash_free(cb_hash_key_t key, cb_hash_value_t value, void* user)
{
    cb_command_future_free((cb_command_future *) value);
    free(value);
}

static void cb_bus_impl_handle_on_message_execution_completed(cb_bus_impl* impl, const TransportMessage* transport_message)
{
    const ProtobufCBinaryData* data = &transport_message->content_bytes;
    MessageExecutionCompleted* message = message_execution_completed__unpack(NULL, data->len, data->data);
    if (message == NULL)
    {
        CB_LOG_DBG(CB_LOG_LEVEL_WARN, "Failed to deserialize MessageExecutionCompleted (%zu bytes)", data->len);
    }
    else
    {
        cb_message_id message_id;
        cb_message_id_from_proto(&message_id, message->source_message_id);
        cb_command_future *command_future = (cb_command_future *)cb_hash_get(impl->message_id_tasks, &message_id);

        char uuid_str[36];
        cb_uuid_print(&message_id.value, uuid_str, sizeof(uuid_str));

        if (command_future == NULL)
        {
            char* originator_endpoint = "Unknown";
            char *originator_peer = "Unknown";

            if (transport_message->originator != NULL)
            {
                originator_endpoint = transport_message->originator->sender_endpoint;
                if (transport_message->originator->sender_id != NULL)
                    originator_peer = transport_message->originator->sender_id->value;
            }

            CB_LOG_DBG(CB_LOG_LEVEL_WARN,
                       "Received unknown response from %s (%s) with id %s",
                       originator_endpoint, originator_peer, uuid_str);
        }
        else
        {
            CB_LOG_DBG(CB_LOG_LEVEL_TRACE, "Setting result for command %s (%s) to %d",
                    message->payload_type_id->full_name, uuid_str, message->error_code);

            command_future->result.error_code = message->error_code;
            if (message->has_payload)
            {
                command_future->result.data.data = message->payload.data;
                command_future->result.data.n_data = message->payload.len;
            }
            cb_future_set(&command_future->future, &command_future->result);
            free(command_future);
        }
    }
}

static void cb_bus_impl_on_transport_message_continuation(cb_message_dispatch* dispatch, void* user)
{
    CB_LOG_DBG(CB_LOG_LEVEL_DEBUG, "Message %s has been handled",
            dispatch->transport_message->message_type_id.value);
}

static void cb_bus_impl_on_transport_message(const TransportMessage* message, void* user)
{
    cb_bus_impl* impl = (cb_bus_impl *)user;
    CB_LOG_DBG(CB_LOG_LEVEL_DEBUG, "Received transport message %s from %s (%s)",
            message->message_type_id->full_name,
            message->originator->sender_endpoint, message->originator->sender_machine);

    if (strcmp(message->message_type_id->full_name, CB_MESSAGE_TYPE_ID_MESSAGE_EXECUTION_COMPLETED) == 0)
    {
        cb_bus_impl_handle_on_message_execution_completed(impl, message);
    }
    else
    {
        cb_transport_message* transport_message = cb_transport_message_from_proto_new(message);
        cb_message_dispatch* dispatch = cb_message_dispatch_new(transport_message, cb_bus_impl_on_transport_message_continuation, impl);
        cb_message_dispatcher_dispatch(&impl->message_dispatcher, dispatch);
    }
}

static cb_peer_id* cb_bus_impl_peer_id(cb_bus* base)
{
    cb_bus_impl* impl = (cb_bus_impl *)base;
    return &impl->peer_id;
}

static cb_bus_error cb_bus_impl_init(cb_bus* base)
{
    cb_bus_impl* impl = (cb_bus_impl *)base;
    if (cb_time_uuid_gen_init_random(&impl->uuid_gen) == cebus_false)
        return cb_bus_error_init;

    impl->is_running = 0;
    impl->message_id_tasks = cb_hash_map_new(cb_message_id_hash, cb_message_id_hash_eq);
    memset(impl->environment, 0, sizeof(impl->environment));

    cb_message_dispatcher_init(&impl->message_dispatcher);
    return cb_bus_ok;
}

static cb_bus_error cb_bus_impl_configure(cb_bus* base, const cb_peer_id* peer_id, const char* environment)
{
    cb_bus_impl* impl = (cb_bus_impl *)base;
    cb_peer_id_set(&impl->peer_id, peer_id->value);
    strncpy(impl->environment, environment, CEBUS_STR_MAX);
    cb_transport_configure(impl->transport, &impl->peer_id, impl->environment);
    return cb_bus_ok;
}

static void cb_bus_impl_register_invoker(cb_bus* base, const cb_message_type_id* message_type_id, const cb_message_handler_invoker* invoker)
{
    cb_bus_impl* impl = (cb_bus_impl *) base;
    cb_message_dispatcher_register(&impl->message_dispatcher, message_type_id, invoker);
}

static cb_future cb_bus_impl_send_to_async(cb_bus* base, cb_command command, const cb_peer* peer)
{
    cb_bus_impl* impl = (cb_bus_impl *)base;
    cb_command_future* command_future = cb_new(cb_command_future, 1);
    cb_command_future_init(command_future);

    cb_transport_message* transport_message = cb_to_transport_message(
            &command,
            &impl->uuid_gen,
            &impl->peer_id,
            impl->inbound_endpoint, 
            impl->environment
    );
    cb_peer_list* peers = cb_peer_list_new();
    cb_transport_error error;
    cb_command_result result;

    cb_command_move(&command_future->command, &command);
    command_future->transport_message = transport_message;

    {
        char uuid_str[36];
        memset(uuid_str, 0, sizeof(uuid_str));
        cb_uuid_print(&transport_message->id.value, uuid_str, sizeof(uuid_str));
        CB_LOG_DBG(CB_LOG_LEVEL_TRACE, "Inserting command task for %s with id %s", 
                transport_message->message_type_id.value, uuid_str);
    }

    cb_hash_insert(impl->message_id_tasks, &transport_message->id, command_future);

    cb_peer_list_add(peers, cb_peer_clone(peer));
    error = cb_transport_send(impl->transport, transport_message, peers);
    if (!CB_TRANSPORT_OK(error))
    {
        fprintf(stderr, "Failed to send transport message: %d\n", error);
        cb_hash_remove(impl->message_id_tasks, &transport_message->id);
    }

    return command_future->future;
}

static cb_command_result cb_bus_impl_send_to(cb_bus* base, cb_command command, const cb_peer* peer)
{
    cb_future future = cb_bus_impl_send_to_async(base, command, peer);
    cb_command_result result;

    cb_command_result_move(&result, cb_future_get(&future));
    cb_future_destroy(&future, NULL);
    return result;
}

static cb_bus_error cb_bus_impl_start(cb_bus* base)
{
    cb_bus_impl* impl = (cb_bus_impl *)base;
    uint64_t expected = 0;
    cb_transport_error transport_err;

    if (cb_atomic_compare_exchange_strong_u64((volatile uint64_t *)&impl->is_running, &expected, 1) == cebus_false)
        return cb_bus_error_already_running;

    CB_LOG_DBG(CB_LOG_LEVEL_DEBUG, "Starting transport ...");

    cb_transport_on_message_received(impl->transport, cb_bus_impl_on_transport_message, impl);
    if (!CB_TRANSPORT_OK(transport_err = cb_transport_start(impl->transport)))
        return cb_bus_error_transport;

    impl->inbound_endpoint = cb_transport_inbound_endpoint(impl->transport);

    cb_message_dispatcher_start(&impl->message_dispatcher);

    return cb_bus_ok;
}

static cb_bus_error cb_bus_impl_stop(cb_bus* base)
{
    cb_bus_impl* impl = (cb_bus_impl *)base;
    uint64_t expected = 1;
    cb_transport_error transport_err;

    if (cb_atomic_compare_exchange_strong_u64((volatile uint64_t *)&impl->is_running, &expected, 0) == cebus_false)
        return cb_bus_error_not_started;

    CB_LOG_DBG(CB_LOG_LEVEL_DEBUG, "Stopping transport ...");
    if  (!CB_TRANSPORT_OK(transport_err = cb_transport_stop(impl->transport)))
        return cb_bus_error_transport;

    cb_message_dispatcher_stop(&impl->message_dispatcher);

    return cb_bus_ok;
}

static void cb_bus_impl_free(cb_bus* base)
{
    cb_bus_impl* impl = (cb_bus_impl *) base;
    cb_hash_map_free(impl->message_id_tasks, cb_bus_impl_message_id_task_hash_free, NULL);
    cb_transport_free(impl->transport);
    cb_message_dispatcher_free(&impl->message_dispatcher);
    free(impl);
}

static void cb_bus_impl_init_base(cb_bus* base)
{
    CB_BUS_VIRT_SET(base, init, cb_bus_impl_init);
    CB_BUS_VIRT_SET(base, peer_id, cb_bus_impl_peer_id);
    CB_BUS_VIRT_SET(base, configure, cb_bus_impl_configure);
    CB_BUS_VIRT_SET(base, register_invoker, cb_bus_impl_register_invoker);
    CB_BUS_VIRT_SET(base, send_to, cb_bus_impl_send_to);
    CB_BUS_VIRT_SET(base, start, cb_bus_impl_start);
    CB_BUS_VIRT_SET(base, stop, cb_bus_impl_stop);
    CB_BUS_VIRT_SET(base, free, cb_bus_impl_free);
}

cb_command_result* cb_command_result_init(cb_command_result* result)
{
    result->error_code = 0;
    cb_buffer_init(&result->data);

    return result;
}

cb_command_result* cb_command_result_copy(cb_command_result* dst, const cb_command_result* src)
{
    dst->error_code = src->error_code;
    cb_buffer_copy(&dst->data, &src->data);

    return dst;
}

cb_command_result* cb_command_result_move(cb_command_result* dst, cb_command_result* src)
{
    dst->error_code = src->error_code;
    cb_buffer_move(&dst->data, &src->data);

    return dst;
}

cb_bus* cb_bus_create(cb_transport* transport)
{
    cb_bus_impl* impl = cb_new(cb_bus_impl, 1);
    cb_bus_impl_init_base(&impl->base);
    impl->transport = transport;
    return &impl->base;
}

cb_bus_error cb_bus_init(cb_bus* bus)
{
    return CB_BUS_VIRT_CALL(bus, init);
}

cb_peer_id* cb_bus_peer_id(cb_bus* bus)
{
    return CB_BUS_VIRT_CALL(bus, peer_id);
}

const char* cb_bus_environment(cb_bus* bus)
{
    return CB_BUS_VIRT_CALL(bus, environment);
}

cebus_bool cb_bus_is_running(cb_bus* bus)
{
    return CB_BUS_VIRT_CALL(bus, is_running);
}

cb_bus_error cb_bus_configure(cb_bus* bus, const cb_peer_id* peer_id, const char* environment)
{
    return CB_BUS_VIRT_CALL(bus, configure, peer_id, environment);
}

void cb_bus_register_invoker(cb_bus* bus, const cb_message_type_id* message_type_id, const cb_message_handler_invoker* invoker)
{
    return CB_BUS_VIRT_CALL(bus, register_invoker, message_type_id, invoker);
}

void cb_bus_publish(cb_bus* bus, const ProtobufCMessage* event)
{
    return CB_BUS_VIRT_CALL(bus, publish, event);
}

cb_command_result cb_bus_send(cb_bus* bus, cb_command command)
{
    return CB_BUS_VIRT_CALL(bus, send, command);
}

cb_command_result cb_bus_send_to(cb_bus* bus, cb_command command, const cb_peer* peer)
{
    return CB_BUS_VIRT_CALL(bus, send_to, command, peer);
}

cb_bus_error cb_bus_start(cb_bus* bus)
{
    return CB_BUS_VIRT_CALL(bus, start);
}

cb_bus_error cb_bus_stop(cb_bus* bus)
{
    return CB_BUS_VIRT_CALL(bus, stop);
}

void cb_bus_free(cb_bus* bus)
{
    return CB_BUS_VIRT_CALL(bus, free);
}
