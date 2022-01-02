#include "cebus/transport/zmq_outbound_socket.h"

#include <string.h>

#define CB_ZMQ_SETSOCKOPT_TRY(socket, option_name, option)                    \
    do {                                                                      \
        if (zmq_setsockopt(socket, option_name, &option, sizeof(option)) < 0) \
            return -1;                                                        \
    } while (0)

static int cb_zmq_outbound_socket_set_options(cb_zmq_outbound_socket* socket)
{
    const cb_zmq_socket_options* options = &socket->options;
    zsock_t* sock                     = socket->sock;
    const uint64_t send_timeout_ms    = (int) timespan_as_millis(options->send_timeout);

    CB_ZMQ_SETSOCKOPT_TRY(sock, ZMQ_SNDHWM  , options->send_high_watermark);
    CB_ZMQ_SETSOCKOPT_TRY(sock, ZMQ_SNDTIMEO, send_timeout_ms);

    if (options->keep_alive.enabled == cebus_true) {
        int keep_alive          = 1;
        int keep_alive_timeout  = timespan_as_secs(options->keep_alive.keep_alive_interval);
        int keep_alive_interval = timespan_as_secs(options->keep_alive.keep_alive_interval);

        CB_ZMQ_SETSOCKOPT_TRY(sock, ZMQ_TCP_KEEPALIVE      , keep_alive);
        CB_ZMQ_SETSOCKOPT_TRY(sock, ZMQ_TCP_KEEPALIVE_IDLE , keep_alive_timeout);
        CB_ZMQ_SETSOCKOPT_TRY(sock, ZMQ_TCP_KEEPALIVE_INTVL, keep_alive_interval);
    }

    return 0;
}

static void cb_zmq_outbound_socket_send_failure(cb_zmq_outbound_socket* socket, int rc)
{
    assert(socket->state == cb_zmq_outbound_socket_state_established);

    socket->cb_zmq_error = rc;

    if (socket->failed_send_count >= socket->options.send_retries_before_switching_to_closed_state)
    {
        socket->state = cb_zmq_outbound_socket_state_closed_wait;
        socket->close_cooldown_timer = time_instant_now();
    }

    ++socket->failed_send_count;
}

static cebus_bool cb_zmq_outbound_socket_can_send(cb_zmq_outbound_socket* socket)
{
    if (socket->state == cb_zmq_outbound_socket_state_closed)
        return cebus_false;

    if (socket->state == cb_zmq_outbound_socket_state_established)
        return cebus_true;
    
    // We are in "closed wait" state, attempt to re-open the socket

    assert(socket->state == cb_zmq_outbound_socket_state_closed_wait);
    {
        timespan elapsed = time_instant_elapsed(&socket->close_cooldown_timer);
        // XXX: check elapsed
    }

    return cebus_false;
}

cb_zmq_outbound_socket* outbound_socket_new(
        void* context,
        const cb_peer_id* peer_id,
        const char* endpoint,
        cb_zmq_socket_options options)
{
    cb_zmq_outbound_socket* socket = malloc(sizeof *socket);
    if (socket == NULL)
        return NULL;

    socket->context = context;
    socket->sock    = NULL;

    cb_peer_id_set(&socket->peer_id, peer_id->value);
    strncpy(socket->endpoint, endpoint, CEBUS_STR_MAX);

    socket->state = cb_zmq_outbound_socket_state_closed;
    socket->options = options;
    return socket;
}

cb_zmq_outbound_socket_error outbound_socket_connect(cb_zmq_outbound_socket* socket)
{
    int rc;
    if (socket->sock == NULL)
    {
        zsock_t* sock = zmq_socket(socket->context, ZMQ_PUSH);
        if (sock == NULL)
            return cb_zmq_outbound_socket_error_sock_create;

        if (cb_zmq_outbound_socket_set_options(socket) < 0)
            return cb_zmq_outbound_socket_error_sock_options;

        socket->sock = sock;
    }

    rc = zmq_connect(socket->sock, socket->endpoint);
    if (rc < 0)
    {
        socket->cb_zmq_error = rc;
        return cb_zmq_outbound_socket_error_connect;
    }

    socket->state = cb_zmq_outbound_socket_state_established;
    socket->cb_zmq_error = 0;
    socket->failed_send_count = 0;
    return cb_zmq_outbound_socket_ok;
}

cb_zmq_outbound_socket_error outbound_socket_disconnect(cb_zmq_outbound_socket* socket)
{
    int rc;
    int linger = 0;

    if (socket->sock == NULL)
        return cb_zmq_outbound_socket_error_not_connected;

    if (socket->state != cb_zmq_outbound_socket_state_established)
        return cb_zmq_outbound_socket_error_not_connected;

    // Set a linger value of 0 which means that pending messages shall be discarded immediately.
    // If this call fails, we do not trigger an error and close the socket anyway.
    zmq_setsockopt(socket->sock, ZMQ_LINGER, &linger, sizeof(linger));

    rc = zmq_close(socket->sock);
    if (rc < 0)
    {
        socket->cb_zmq_error = rc;
        return cb_zmq_outbound_socket_error_close;
    }

    socket->sock = NULL;
    socket->state = cb_zmq_outbound_socket_state_closed;
    return cb_zmq_outbound_socket_ok;
}

cb_zmq_outbound_socket_error outbound_socket_send(cb_zmq_outbound_socket *socket, const void* data, size_t size)
{
    int rc;

    if (socket == NULL)
        return cb_zmq_outbound_socket_error_not_connected;

    if (socket->sock == NULL)
        return cb_zmq_outbound_socket_error_not_connected;

    if (!cb_zmq_outbound_socket_can_send(socket))
        return cb_zmq_outbound_socket_error_send_cooldown;

    zframe_t* frame = zframe_new(data, size);
    rc = zframe_send(&frame, socket->sock, 0);
    if (rc == 0)
        return cb_zmq_outbound_socket_ok;

    socket->cb_zmq_error = rc;
    cb_zmq_outbound_socket_send_failure(socket, rc);

    return cb_zmq_outbound_socket_error_send;
}
