#pragma once

#include "cebus/peer_id.h"
#include "cebus/cebus_bool.h"

#include "cebus/transport/zmq_socket_options.h"
#include "cebus/utils/time.h"

#include <czmq.h>

typedef enum cb_cb_zmq_outbound_socket_error
{
    /// No error
    cb_zmq_outbound_socket_ok = 0,

    /// An error occured when creating the 0MQ socket
    cb_zmq_outbound_socket_error_sock_create,

    /// An error occurend when setting the socket options
    cb_zmq_outbound_socket_error_sock_options,

    /// An error occured when connecting the socket
    cb_zmq_outbound_socket_error_connect,

    /// An error occured when closing the socket
    cb_zmq_outbound_socket_error_close,

    /// An error occured when attempting an operation on the socket while it was not connected
    cb_zmq_outbound_socket_error_not_connected,

    /// The connection is in "closed_wait" state and is waiting for the `closed_state_duration_after_send_failure`
    /// cooldown period before attempting to send again
    cb_zmq_outbound_socket_error_send_cooldown,

    /// An error occured when sending a frame
    cb_zmq_outbound_socket_error_send,
} cb_zmq_outbound_socket_error;

/// The state of the 0MQ connection
typedef enum cb_zmq_outbound_socket_state
{
    /// The connection has been "established"
    cb_zmq_outbound_socket_state_established,

    /// The connection has been logically "closed" after reaching the HWM
    /// and is waiting for the `closed_state_duration_after_send_failure` delay
    /// before re-opening it
    cb_zmq_outbound_socket_state_closed_wait,

    /// The connection has been closed
    cb_zmq_outbound_socket_state_closed
} cb_zmq_outboud_socket_state;

typedef struct cb_zmq_outbound_socket
{
    void* context;
    zsock_t *sock;

    cb_zmq_outboud_socket_state state;

    cb_peer_id peer_id;
    char endpoint[CEBUS_STR_MAX];

    int cb_zmq_error;

    cb_zmq_socket_options options;

    size_t failed_send_count;
    time_instant close_cooldown_timer;
} cb_zmq_outbound_socket;

/// Create a new ZMQ outbound socket. Return `NULL` on failure
cb_zmq_outbound_socket *cb_zmq_outbound_socket_new(void* context, const cb_peer_id* peer_id, const char* endpoint, cb_zmq_socket_options options);

/// Connect the socket to the configured endpoint.
/// Returns `cb_zmq_outbound_socket_ok` if success or a `cb_zmq_outbound_socket_error` if failure.
cb_zmq_outbound_socket_error cb_zmq_outbound_socket_connect(cb_zmq_outbound_socket* sock);

/// Disconnect the socket from the configured endpoint.
/// Returns `cb_zmq_outbound_socket_ok` if success or a `cb_zmq_outbound_socket_error` if failure.
cb_zmq_outbound_socket_error cb_zmq_outbound_socket_disconnect(cb_zmq_outbound_socket* socket);

cb_zmq_outbound_socket_error cb_zmq_outbound_socket_send(cb_zmq_outbound_socket* sock, const void* data, size_t size);
