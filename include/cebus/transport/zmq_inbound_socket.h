#pragma once

#include "cebus/config.h"
#include "cebus/peer_id.h"

#include "cebus/transport/zmq.h"
#include "cebus/transport/zmq_socket_options.h"

typedef enum cb_zmq_inbound_socket_error
{
    /// No error
    cb_zmq_inbound_socket_ok = 0,
    //
    /// The argument supplied was invalid
    cb_zmq_inbound_socket_error_invalid,

    /// An error occured when creating the 0MQ socket
    cb_zmq_inbound_socket_error_sock_create,

    /// An error occurend when setting the socket options
    cb_zmq_inbound_socket_error_sock_options,

    /// The endpoint that has been bound by 0MQ is too large
    cb_zmq_inbound_socket_error_endpoint_too_large,

    /// An error occured while attempting to bind the 0MQ socket to the configured endpoint
    cb_zmq_inbound_socket_error_bind,

    /// An error occured while receiving data
    cb_zmq_inbound_socket_error_recv,

    /// An error occured while attempting to unbind the 0MQ socket to the configured endpoint
    cb_zmq_inbound_socket_error_unbind,
} cb_zmq_inbound_socket_error;

typedef struct cb_zmq_inbound_socket
{
    /// The 0MQ context
    void* context;

    /// The underlying 0MQ socket
    zsock_t* sock;

    /// The endpoint to bind to
    char endpoint[CEBUS_ENDPOINT_MAX];

    /// The endpoint that has been bound to by 0MQ
    char zmq_endpoint[CEBUS_ENDPOINT_MAX];

    /// The ID of our peer
    cb_peer_id peer_id; 

    /// The last 0MQ error
    int zmq_error;

    /// The options of the underlying 0MQ socket
    cb_zmq_socket_options options;
} cb_zmq_inbound_socket;

/// Create a new 0MQ inbound socket or `NULL` on failure
cb_zmq_inbound_socket* cb_zmq_inbound_socket_new(
        void* context, const cb_peer_id* peer_id, const char* endpoint, cb_zmq_socket_options options);

/// Bind the underlying 0MQ `socket` to the configured endpoint
/// Return `cb_zmq_inbound_socket_error_ok` if success or a `cb_inbound_socket_error` if failure
cb_zmq_inbound_socket_error cb_zmq_inbound_socket_bind(cb_zmq_inbound_socket* socket);

/// Get the underlying 0MQ endpoint
const char* cb_zmq_inbound_socket_endpoint(const cb_zmq_inbound_socket* socket);

/// Close the underlying 0MQ `socket`
/// Return `cb_zmq_inbound_socket_error_ok` if success or a `cb_inbound_socket_error` if failure
cb_zmq_inbound_socket_error cb_zmq_inbound_socket_close(cb_zmq_inbound_socket* socket);

/// Receive up to `size` bytes from the 0MQ `socket` into the `buffer`
/// Return `cb_zmq_inbound_socket_error_ok` if success and fill the `n_bytes` output parameter if success
/// or a `cb_inbound_socket_error` if failure
cb_zmq_inbound_socket_error cb_zmq_inbound_socket_read(cb_zmq_inbound_socket* socket, zmq_msg_t* msg);

/// Release memory owned by the `socket`
void cb_zmq_inbound_socket_free(cb_zmq_inbound_socket* socket);
