#pragma once

#include "cebus/peer_id.h"
#include "cebus/cebus_bool.h"

#include "cebus/transport/zmq_socket_options.h"
#include "cebus/utils/time.h"

#include <czmq.h>

typedef enum zmq_outbound_socket_error
{
    /// No error
    zmq_outbound_socket_ok = 0,

    /// An error occured when creating the 0MQ socket
    zmq_outbound_socket_error_sock_create,

    /// An error occurend when setting the socket options
    zmq_outbound_socket_error_sock_options,

    /// An error occured when connecting the socket
    zmq_outbound_socket_error_connect,

    /// An error occured when closing the socket
    zmq_outbound_socket_error_close,

    /// An error occured when attempting an operation on the socket while it was not connected
    zmq_outbound_socket_error_not_connected,

    /// The connection is in "closed_wait" state and is waiting for the `closed_state_duration_after_send_failure`
    /// cooldown period before attempting to send again
    zmq_outbound_socket_error_send_cooldown,

    /// An error occured when sending a frame
    zmq_outbound_socket_error_send,
} zmq_outbound_socket_error;

/// The state of the 0MQ connection
typedef enum zmq_outbound_socket_state
{
    /// The connection has been "established"
    zmq_outbound_socket_state_established,

    /// The connection has been logically "closed" after reaching the HWM
    /// and is waiting for the `closed_state_duration_after_send_failure` delay
    /// before re-opening it
    zmq_outbound_socket_state_closed_wait,

    /// The connection has been closed
    zmq_outbound_socket_state_closed
} zmq_outboud_socket_state;

typedef struct zmq_outbound_socket
{
    void* context;
    zsock_t *sock;

    zmq_outboud_socket_state state;

    peer_id peer_id;
    char endpoint[CEBUS_STR_MAX];

    int zmq_error;

    zmq_socket_options options;

    size_t failed_send_count;
    time_instant close_cooldown_timer;
} zmq_outbound_socket;

/// Create a new ZMQ outbound socket. Return `NULL` on failure
zmq_outbound_socket *outbound_socket_new(void* context, const peer_id* peer_id, const char* endpoint, zmq_socket_options options);

/// Connect the socket to the configured endpoint.
/// Returns `zmq_outbound_socket_ok` if success or a `zmq_outbound_socket_error` if failure.
zmq_outbound_socket_error outbound_socket_connect(zmq_outbound_socket* sock);

/// Disconnect the socket from the configured endpoint.
/// Returns `zmq_outbound_socket_ok` if success or a `zmq_outbound_socket_error` if failure.
zmq_outbound_socket_error outbound_socket_disconnect(zmq_outbound_socket* socket);

zmq_outbound_socket_error outbound_socket_send(zmq_outbound_socket* sock, const void* data, size_t size);

void outbound_socket_close(zmq_outbound_socket* sock);
