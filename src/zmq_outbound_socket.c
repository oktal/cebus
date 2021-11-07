#include "cebus/transport/zmq_outbound_socket.h"

#include <string.h>

zmq_outbound_socket* outbound_socket_new(void* context, const peer_id* peer_id, const char* endpoint)
{
    zmq_outbound_socket* socket = malloc(sizeof *socket);
    if (socket == NULL)
        return NULL;

    socket->context = context;
    socket->sock = NULL;
    peer_id_set(&socket->peer_id, peer_id->value);
    strncpy(socket->endpoint, endpoint, CEBUS_STR_MAX);

    return socket;
}

zmq_outbound_socket_error outbound_socket_connect(zmq_outbound_socket* socket)
{
    int res;
    if (socket->sock == NULL)
    {
        zsock_t* sock = zmq_socket(socket->context, ZMQ_PUSH);
        if (sock == NULL)
            return zmq_outbound_socket_error_sock_create;


        // zsock_set_identity(sock, peer_id_get(&socket->peer_id));
        // @todo: KeepAlive options
        socket->sock = sock;
    }

    res = zmq_connect(socket->sock, socket->endpoint);
    if (res < 0)
        return zmq_outbound_socker_error_connect;

    socket->connected = cebus_true;
    return zmq_outbound_socket_ok;
}

zmq_outbound_socket_error outbound_socket_send(zmq_outbound_socket *socket, const void* data, size_t size)
{
    int rc;

    if (socket == NULL)
        return zmq_outbound_socket_error_not_connected;

    if (socket->sock == NULL)
        return zmq_outbound_socket_error_not_connected;

    if (socket->connected == cebus_false)
        return zmq_outbound_socket_error_not_connected;

    zframe_t* frame = zframe_new(data, size);
    rc = zframe_send(&frame, socket->sock, 0);
    if (rc < 0)
    {
        socket->zmq_error = rc;
        return zmq_outbound_socket_error_send;
    }

    return zmq_outbound_socket_ok;
}

void outbound_socket_close(zmq_outbound_socket* socket)
{
    zmq_close(socket);
}
