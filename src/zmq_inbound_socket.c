#include "cebus/transport/zmq_inbound_socket.h"

#include "cebus/alloc.h"

static int cb_zmq_inbound_socket_set_options(zsock_t* sock, const cb_zmq_socket_options* options)
{
    const int recv_timeout_ms       = (int) timespan_as_millis(options->receive_timeout);

    CB_ZMQ_SETSOCKOPT_TRY(sock, ZMQ_RCVHWM  , options->receive_high_watermark);
    CB_ZMQ_SETSOCKOPT_TRY(sock, ZMQ_RCVTIMEO, recv_timeout_ms);

    return 0;
}

cb_zmq_inbound_socket* cb_zmq_inbound_socket_new(
    void* context, const cb_peer_id* peer_id, const char* endpoint, cb_zmq_socket_options options)
{
    cb_zmq_inbound_socket* socket = cb_alloc(cb_zmq_inbound_socket, 1);

    socket->context   = context;
    socket->sock      = NULL;
    socket->zmq_error = 0;

    cb_peer_id_set(&socket->peer_id, peer_id->value);
    strncpy(socket->endpoint, endpoint, CEBUS_ENDPOINT_MAX);

    socket->options = options;
    return socket;
}

cb_zmq_inbound_socket_error cb_zmq_inbound_socket_bind(cb_zmq_inbound_socket* socket)
{
    int rc;
    if (socket->sock == NULL)
    {
        zsock_t* sock = zmq_socket(socket->context, ZMQ_PULL);
        if (sock == NULL)
            return cb_zmq_inbound_socket_error_sock_create;

        if ((rc = cb_zmq_inbound_socket_set_options(sock, &socket->options)) < 0)
        {
            socket->zmq_error = errno;
            return cb_zmq_inbound_socket_error_sock_options;
        }

        socket->sock = sock;
    }

    if ((rc = zmq_bind(socket->sock, socket->endpoint)) < 0)
    {
        socket->zmq_error = errno;
        return cb_zmq_inbound_socket_error_bind;
    }

    return cb_zmq_inbound_socket_ok;
}

cb_zmq_inbound_socket_error cb_zmq_inbound_socket_close(cb_zmq_inbound_socket* socket)
{
    int rc;

    if (socket->sock == NULL)
        return cb_zmq_inbound_socket_error_invalid;

    if ((rc = zmq_unbind(socket->sock, socket->endpoint)) < 0)
    {
        socket->zmq_error = errno;
        return cb_zmq_inbound_socket_error_unbind;
    }

    return cb_zmq_inbound_socket_ok;
}

cb_zmq_inbound_socket_error cb_zmq_inbound_socket_read(cb_zmq_inbound_socket* socket, zmq_msg_t* msg)
{
    int rc = 0;
    if (socket->sock == NULL)
        return cb_zmq_inbound_socket_error_invalid;

    while ((rc = zmq_recvmsg(socket->sock, msg, 0)) == -1)
    {
        if (errno == EINTR)
            continue;

        socket->zmq_error = errno;
        return cb_zmq_inbound_socket_error_recv;
    }

    return cb_zmq_inbound_socket_ok;
}

void cb_zmq_inbound_socket_free(cb_zmq_inbound_socket* socket)
{
    if (socket->sock != NULL)
        cb_zmq_inbound_socket_close(socket);

    free(socket);
}
