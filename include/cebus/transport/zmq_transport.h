#pragma once

#include "cebus/collection/hash_map.h"
#include "cebus/peer.h"
#include "cebus/peer_id.h"
#include "cebus/threading.h"
#include "cebus/transport_message.h"

#include "cebus/transport/zmq_inbound_socket.h"
#include "cebus/transport/zmq_outbound_socket.h"
#include "cebus/transport/zmq_socket_options.h"
#include "cebus/transport/zmq_transport_configuration.h"

#include <pthread.h>

typedef enum cb_zmq_transport_error
{
    /// Success
    cb_zmq_transport_ok,

    /// An error occured when starting the inbound thread
    cb_zmq_transport_error_start_inbound,

    /// An error occured when starting the outbound thread
    cb_zmq_transport_error_start_outbound,
} cb_zmq_transport_error;

typedef void (*cb_zmq_transport_on_message)(const TransportMessage* message);

typedef struct cb_zmq_transport
{
    cb_zmq_socket_options socket_options;
    cb_zmq_transport_configuration configuration;

    cb_zmq_inbound_socket* socket;
    cb_hash_map* outbound_sockets;

    cb_peer_id peer_id;
    char environment[CEBUS_STR_MAX];

    char inbound_endpoint[CEBUS_ENDPOINT_MAX];

    cb_thread inbound_thread;
    cb_thread outbound_thread;

    pthread_mutex_t outbound_action_mutex;
    pthread_cond_t outbound_action_cond;
    struct cb_zmq_outbound_action* outbound_action_head;

    cb_zmq_transport_on_message on_message;

    void *zmq_context;
} cb_zmq_transport;

typedef struct cb_peer_entry
{
    cb_peer* peer;
    struct cb_peer_entry* next;
} cb_peer_entry;

typedef struct cb_peer_list
{
    cb_peer_entry* head;
    cb_peer_entry* tail;
} cb_peer_list;

cb_peer_list* cb_peer_list_new();
void cb_peer_list_add(cb_peer_list* list, cb_peer* peer);
void cb_peer_list_free(cb_peer_list* list);

cb_zmq_transport* cb_zmq_transport_new(cb_zmq_transport_configuration configuration, cb_zmq_socket_options socket_options, cb_zmq_transport_on_message on_message);
void cb_zmq_transport_configure(cb_zmq_transport* transport, const cb_peer_id *peer_id, const char* environment);
void cb_zmq_transport_free(cb_zmq_transport* transport);

cb_zmq_transport_error cb_zmq_transport_start(cb_zmq_transport* transport);
cb_zmq_transport_error cb_zmq_transport_stop(cb_zmq_transport* transport);

cb_zmq_transport_error cb_zmq_transport_send(
        cb_zmq_transport* transport, cb_transport_message* transport_message, cb_peer_list* peers);

void cb_zmq_transport_free(cb_zmq_transport* transport);
