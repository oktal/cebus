//! This file defines the `bus` interface from which the user can send or subscribe to commands and events

#pragma once

#include "cebus/cebus_bool.h"

#include "cebus/buffer.h"
#include "cebus/command.h"
#include "cebus/config.h"
#include "cebus/dispatch/message_dispatcher.h"
#include "cebus/peer.h"
#include "cebus/peer_id.h"
#include "cebus/threading.h"

#include "cebus/transport/transport.h"

#define CB_BUS_VA_ARGS(...) , ##__VA_ARGS__
#define CB_BUS_VIRTUAL(ret, func_name, ...) \
    ret (*func_name##_func)(struct cb_bus* bus CB_BUS_VA_ARGS(__VA_ARGS__))

typedef enum cb_bus_error
{
    /// No error
    cb_bus_ok,

    /// An error occured while initializing internal bus components
    cb_bus_error_init,

    /// Bus is already running
    cb_bus_error_already_running,

    /// Bus has not been started
    cb_bus_error_not_started,

    /// An error occured when starting the underlying transport layer
    cb_bus_error_transport,
} cb_bus_error;

typedef struct cb_command_result
{
    /// The error code of the command
    int error_code;

    cb_buffer data;
} cb_command_result;

/// Initialize a new `cb_command_result`
/// Return `result`
cb_command_result* cb_command_result_init(cb_command_result* result);

/// Copy a `cb_command_result` from `src` to `dst`
/// Return `dst`
cb_command_result* cb_command_result_copy(cb_command_result* dst, const cb_command_result* src);

/// Move a `cb_command_result` from `src` to `dst`. A move operation will invalidate `src`
/// Return `dst`
cb_command_result* cb_command_result_move(cb_command_result* dst, cb_command_result* src);

/// The bus interface
typedef struct cb_bus
{
    CB_BUS_VIRTUAL(cb_bus_error, init);

    CB_BUS_VIRTUAL(cb_peer_id *, peer_id);
    CB_BUS_VIRTUAL(const char*, environment);
    CB_BUS_VIRTUAL(cebus_bool, is_running);

    CB_BUS_VIRTUAL(cb_bus_error, configure, const cb_peer_id* peer_id, const char *environment);
    CB_BUS_VIRTUAL(void, register_invoker, const cb_message_type_id* message_type_id, const cb_message_handler_invoker* invoker);

    CB_BUS_VIRTUAL(void, publish, const ProtobufCMessage* event);

    CB_BUS_VIRTUAL(cb_future, send_async, cb_command command);
    CB_BUS_VIRTUAL(cb_future, send_to_async, cb_command command, const cb_peer* peer);

    CB_BUS_VIRTUAL(cb_command_result, send, cb_command command);
    CB_BUS_VIRTUAL(cb_command_result, send_to, cb_command command, const cb_peer* peer);

    CB_BUS_VIRTUAL(cb_bus_error, start);
    CB_BUS_VIRTUAL(cb_bus_error, stop);

    CB_BUS_VIRTUAL(void, free);
} cb_bus;

#undef CB_BUS_VIRTUAL
#undef CB_BUS_VA_ARGS

/// Create a new default bus with the given `transport` layer
cb_bus* cb_bus_create(cb_transport* transport);

/// Initialize the given `bus`. Return `cb_bus_ok` if initialization succeeded or `cb_bus_error` otherwise
cb_bus_error cb_bus_init(cb_bus* bus);

/// Get the peer id associated to the given `bus`
cb_peer_id* cb_bus_peer_id(cb_bus* bus);

/// Get the environment associated to the given `bus`
const char* cb_bus_environment(cb_bus* bus);

/// Return whether the `bus` is currently running or not
cebus_bool cb_bus_is_running(cb_bus* bus);

/// Configure the `bus` with the given `peer_id` and `environment`. Return `cb_bus_ok` on success or `cb_bus_error` otherwise
cb_bus_error cb_bus_configure(cb_bus* bus, const cb_peer_id* peer_id, const char* environment);

/// Register the message invoker for a `cb_message_type_id` to the given `bus`
void cb_bus_register_invoker(cb_bus* bus, const cb_message_type_id* message_type_id, const cb_message_handler_invoker* invoker);

/// Publish the given `event` on the `bus`
void cb_bus_publish(cb_bus* bus, const ProtobufCMessage* event);

/// Send the given `command` to the corresponding peer handling it
/// This send is asynchronous
cb_future cb_bus_send_async(cb_bus* bus, cb_command command);

/// Send the given `command` to a specific target peer.
/// This send is asynchronous
cb_future cb_bus_send_to_async(cb_bus* bus, cb_command command, const cb_peer* peer);

/// Send the given `command` to the corresponding peer handling it
cb_command_result cb_bus_send(cb_bus* bus, cb_command command);

/// Send the given `command` to a specific target `peer`
cb_command_result cb_bus_send_to(cb_bus* bus, cb_command command, const cb_peer* peer);

/// Start the bus. Return `cb_bus_ok` if the bus successfully started or `cb_bus_error` otherwise
cb_bus_error cb_bus_start(cb_bus* bus);
//
/// Stop the bus. Return `cb_bus_ok` if the bus successfully stopped or `cb_bus_error` otherwise
cb_bus_error cb_bus_stop(cb_bus* bus);

/// Free the ressources owned by the given `bus`
void cb_bus_free(cb_bus* bus);
