//! This file defines the `bus` interface from which the user can send or subscribe to commands and events

#pragma once

#include "cebus/cebus_bool.h"

#include "cebus/config.h"
#include "cebus/peer.h"
#include "cebus/peer_id.h"

#include "cebus/transport/transport.h"

#include <protobuf-c/protobuf-c.h>

#define CB_BUS_VA_ARGS(...) , ##__VA_ARGS__
#define CB_BUS_VIRTUAL(ret, func_name, ...) \
    ret (*func_name##_func)(struct cb_bus* bus CB_BUS_VA_ARGS(__VA_ARGS__))

typedef struct cb_command_result
{
} cb_command_result;

typedef enum cb_bus_error
{
    /// No error
    cb_bus_ok,

    /// Bus is already running
    cb_bus_error_already_running,

    /// An error occured when starting the underlying transport layer
    cb_bus_error_transport,
} cb_bus_error;

/// The bus interface
typedef struct cb_bus
{
    CB_BUS_VIRTUAL(cb_peer_id *, peer_id);
    CB_BUS_VIRTUAL(const char*, environment);
    CB_BUS_VIRTUAL(cebus_bool, is_running);

    CB_BUS_VIRTUAL(void, configure, const cb_peer_id* peer_id, const char *environment);
    CB_BUS_VIRTUAL(void, publish, const ProtobufCMessage* event);

    CB_BUS_VIRTUAL(cb_command_result, send, const ProtobufCMessage* message);
    CB_BUS_VIRTUAL(cb_command_result, send_to, const ProtobufCMessage* message, const cb_peer* peer);

    CB_BUS_VIRTUAL(cb_bus_error, start);
    CB_BUS_VIRTUAL(cb_bus_error, stop);

    CB_BUS_VIRTUAL(void, free);
} cb_bus;

#undef CB_BUS_VIRTUAL
#undef CB_BUS_VA_ARGS

/// Create a new default bus with the given `transport` layer
cb_bus* cb_bus_create(cb_transport* transport);

/// Get the peer id associated to the given `bus`
cb_peer_id* cb_bus_peer_id(cb_bus* bus);

/// Get the environment associated to the given `bus`
const char* cb_bus_environment(cb_bus* bus);

/// Return whether the `bus` is currently running or not
cebus_bool cb_bus_is_running(cb_bus* bus);

/// Configure the `bus` with the given `peer_id` and `environment`
void cb_bus_configure(cb_bus* bus, const cb_peer_id* peer_id, const char* environment);

/// Publish the given `event` on the `bus`
void cb_bus_publish(cb_bus* bus, const ProtobufCMessage* event);

/// Send the given `message` to the corresponding peer handling it
cb_command_result cb_bus_send(cb_bus* bus, const ProtobufCMessage* message);

/// Send the given `message` to a specific target `peer`
cb_command_result cb_bus_send_to(cb_bus* bus, const ProtobufCMessage* message, const cb_peer* peer);

/// Start the bus. Return `cb_bus_ok` if the bus successfully started of `cb_bus_error` otherwise
cb_bus_error cb_bus_start(cb_bus* bus);
//
/// Stop the bus. Return `cb_bus_ok` if the bus successfully stopped of `cb_bus_error` otherwise
cb_bus_error cb_bus_stop(cb_bus* bus);

/// Free the ressources owned by the given `bus`
void cb_bus_free(cb_bus* bus);
