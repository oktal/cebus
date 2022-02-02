#include "cebus/bus.h"

#include "cebus/alloc.h"
#include "cebus/atomic.h"
#include "cebus/collection/hash_map.h"
#include "cebus/log.h"
#include "cebus/message_id.h"
#include "cebus/threading.h"

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
    cebus_bool is_running;

    // Current peer id
    cb_peer_id peer_id;

    // Current environment
    char environment[CEBUS_STR_MAX];
} cb_bus_impl;

static void cb_bus_impl_on_transport_message(const TransportMessage* message, void* user)
{
    cb_bus_impl* impl = (cb_bus_impl *)user;
    CB_LOG_DBG(CB_LOG_LEVEL_DEBUG, "Received transport message from %s (%s)",
            message->originator->sender_endpoint, message->originator->sender_machine);
}

static cb_peer_id* cb_bus_impl_peer_id(cb_bus* base)
{
    cb_bus_impl* impl = (cb_bus_impl *)base;
    return &impl->peer_id;
}

static void cb_bus_impl_configure(cb_bus* base, const cb_peer_id* peer_id, const char* environment)
{
    cb_bus_impl* impl = (cb_bus_impl *)base;
    cb_peer_id_set(&impl->peer_id, peer_id->value);
    strncpy(impl->environment, environment, CEBUS_STR_MAX);
    cb_transport_configure(impl->transport, &impl->peer_id, impl->environment);
}

static cb_bus_error cb_bus_impl_start(cb_bus* base)
{
    cb_bus_impl* impl = (cb_bus_impl *)base;
    uint64_t expected = (uint64_t) cebus_false;
    cb_transport_error transport_err;

    if (cb_atomic_compare_exchange_strong_u64((volatile uint64_t *)&impl->is_running, expected, cebus_true) == cebus_false)
        return cb_bus_error_already_running;

    CB_LOG_DBG(CB_LOG_LEVEL_DEBUG, "Starting transport ...");

    cb_transport_on_message_received(impl->transport, cb_bus_impl_on_transport_message, impl);
    if (!CB_TRANSPORT_OK(transport_err = cb_transport_start(impl->transport)))
    {
        CB_LOG_DBG(CB_LOG_LEVEL_ERROR, "Error starting transport");
        return cb_bus_error_transport;
    }

    return cb_bus_ok;
}

static void cb_bus_impl_free(cb_bus* base)
{
    cb_bus_impl* impl = (cb_bus_impl *) base;
    cb_hash_map_free(impl->message_id_tasks);
    cb_transport_free(impl->transport);
    free(impl);
}

static void cb_bus_impl_init_base(cb_bus* base)
{
    CB_BUS_VIRT_SET(base, peer_id, cb_bus_impl_peer_id);
    CB_BUS_VIRT_SET(base, configure, cb_bus_impl_configure);
    CB_BUS_VIRT_SET(base, start, cb_bus_impl_start);
    CB_BUS_VIRT_SET(base, free, cb_bus_impl_free);
}

cb_bus* cb_bus_create(cb_transport* transport)
{
    cb_bus_impl* impl = cb_new(cb_bus_impl, 1);
    cb_bus_impl_init_base(&impl->base);
    impl->message_id_tasks = cb_hash_map_new(cb_message_id_hash, cb_message_id_hash_eq);
    impl->transport = transport;
    impl->is_running = cebus_false;
    return &impl->base;
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

void cb_bus_configure(cb_bus* bus, const cb_peer_id* peer_id, const char* environment)
{
    return CB_BUS_VIRT_CALL(bus, configure, peer_id, environment);
}

void cb_bus_publish(cb_bus* bus, const ProtobufCMessage* event)
{
    return CB_BUS_VIRT_CALL(bus, publish, event);
}

cb_command_result cb_bus_send(cb_bus* bus, const ProtobufCMessage* message)
{
    return CB_BUS_VIRT_CALL(bus, send, message);
}

cb_command_result cb_bus_send_to(cb_bus* bus, const ProtobufCMessage* message, const cb_peer* peer)
{
    return CB_BUS_VIRT_CALL(bus, send_to, message, peer);
}

cb_bus_error cb_bus_start(cb_bus* bus)
{
    return CB_BUS_VIRT_CALL(bus, start);
}

void cb_bus_free(cb_bus* bus)
{
    return CB_BUS_VIRT_CALL(bus, free);
}
