#pragma once

#include "cebus/peer_id.h"
#include "cebus/cebus_bool.h"

#include <czmq.h>

#define ENDPOINT_MAX 255

typedef enum zmq_outbound_socket_error
{
    zmq_outbound_socket_ok = 0,
    zmq_outbound_socket_error_sock_create,
    zmq_outbound_socker_error_connect,

    zmq_outbound_socket_error_not_connected,
    zmq_outbound_socket_error_send,
} zmq_outbound_socket_error;

typedef struct zmq_outbound_socket
{
    void* context;
    zsock_t *sock;

    cebus_bool connected;

    peer_id peer_id;
    char endpoint[CEBUS_STR_MAX];

    int zmq_error;
} zmq_outbound_socket;

zmq_outbound_socket *outbound_socket_new(void* context, const peer_id* peer_id, const char* endpoint);
zmq_outbound_socket_error outbound_socket_connect(zmq_outbound_socket* sock);
zmq_outbound_socket_error outbound_socket_send(zmq_outbound_socket* sock, const void* data, size_t size);

void outbound_socket_close(zmq_outbound_socket* sock);
