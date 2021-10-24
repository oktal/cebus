#include <stdio.h>
#include <string.h>
#include <czmq.h>

#include "peer_id.h"

static int get_peer_routing_id(const peer_id* peerid)
{
    int routing_id = 0;
    const char* s = peer_id_get(peerid);
    const size_t len = strlen(s);
    int i;

    for (i = 0; i < len; ++i)
    {
        routing_id += (int) s[i];
    }
    return routing_id;
}

int main()
{
    /*
    void* context = zmq_ctx_new();
    void* outbound_socket = zmq_socket(context, ZMQ_PUSH);
    zmq_connect(outbound_socket, "tcp://localhost:9090");
    zmq_send(outbound_socket, "Hello", 5, 0);

    zmq_close(outbound_socket);
    zmq_ctx_destroy(context);
    */

    return 0;
}
