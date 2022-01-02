#include "cebus/transport/zmq_transport.h"

#include "cebus/alloc.h"
#include "cebus/peer.h"
#include "cebus/peer_id.h"
#include "cebus/transport_message.h"

#include <czmq.h>

typedef struct cb_peer_entry
{
    cb_peer* peer;
    struct cb_peer_entry* next;
} cb_peer_entry;

typedef struct cb_zmq_outbound_action_send
{
    cb_transport_message* message;

    cb_peer_entry* target_peers;
} cb_zmq_outbound_action_send;

typedef struct cb_zmq_outbound_action_disconnect
{
    cb_transport_message* message;

    cb_peer* peer;
} cb_zmq_outbound_send_action_disconnect;


typedef enum cb_zmq_outbound_action_type
{
    cb_zmq_outbound_action_type_send,

    cb_zmq_outbound_action_type_disconnect,
} cb_zmq_outbound_action_type;

typedef struct cb_zmq_outbound_action
{
    union
    {
        cb_zmq_outbound_action_send send;

        cb_zmq_outbound_send_action_disconnect disconnect;
    } action;

    cb_zmq_outbound_action_type type;
} cb_zmq_outbound_action;

typedef struct cb_zmq_outbound_action_entry
{
    cb_zmq_outbound_action* next;
} cb_zmq_outbound_action_entry;

static void* cb_zmq_transport_create_zmq_context(const cb_zmq_socket_options* options)
{
    void* context = zmq_ctx_new();
    if (options->maximum_socket_count > 0)
        zmq_ctx_set(context, ZMQ_MAX_SOCKETS, options->maximum_socket_count);

    return context;
}

static void* cb_zmq_transport_inbound_loop(void* arg)
{
    cb_zmq_transport *transport = (cb_zmq_transport *) arg;

    return NULL;
}

static void* cb_zmq_transport_outbound_loop(void* arg)
{
    cb_zmq_transport *transport = (cb_zmq_transport *) arg;

    return NULL;
}

cb_zmq_transport* cb_zmq_transport_new(
        cb_zmq_transport_configuration configuration, cb_zmq_socket_options socket_options)
{
    cb_zmq_transport* transport = cb_alloc(cb_zmq_transport, 1);
    cb_hash_map* peers = cb_hash_map_new(cb_peer_id_hash, cb_peer_id_hash_eq);

    transport->configuration = configuration;
    transport->socket_options = socket_options;
    transport->peers = peers;
    transport->zmq_context = NULL;

    cb_thread_init(&transport->inbound_thread);
    cb_thread_init(&transport->outbound_thread);

    return transport;
}

cb_zmq_transport_error cb_zmq_transport_start(cb_zmq_transport* transport)
{
    transport->zmq_context = cb_zmq_transport_create_zmq_context(&transport->socket_options);
    cebus_bool start_res;

    if ((start_res = cb_thread_spawn(&transport->inbound_thread, cb_zmq_transport_inbound_loop, transport)) == cebus_false)
        return cb_zmq_transport_error_start_inbound;

    if ((start_res = cb_thread_spawn(&transport->outbound_thread, cb_zmq_transport_outbound_loop, transport)) == cebus_false)
        return cb_zmq_transport_error_start_outbound;

    return cb_zmq_transport_ok;
}

cb_zmq_transport_error cb_zmq_transport_stop(cb_zmq_transport* transport)
{
    return cb_zmq_transport_ok;
}

void cb_zmq_transport_free(cb_zmq_transport* transport)
{
    cb_hash_map_free(transport->peers);
    zmq_ctx_destroy(transport->zmq_context);
    free(transport);
}
