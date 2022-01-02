#pragma once

#include "cebus/collection/hash_map.h"
#include "cebus/peer_id.h"
#include "cebus/threading.h"

#include "cebus/transport/zmq_outbound_socket.h"
#include "cebus/transport/zmq_socket_options.h"
#include "cebus/transport/zmq_transport_configuration.h"

typedef enum cb_zmq_transport_error
{
    /// Success
    cb_zmq_transport_ok,

    /// An error occured when starting the inbound thread
    cb_zmq_transport_error_start_inbound,

    /// An error occured when starting the outbound thread
    cb_zmq_transport_error_start_outbound,
} cb_zmq_transport_error;

struct cb_zmq_outbound_action_entry;

typedef struct cb_zmq_transport
{
    cb_zmq_socket_options socket_options;
    cb_zmq_transport_configuration configuration;

    cb_hash_map* peers;

    cb_peer_id self_id;
    char environment[CEBUS_STR_MAX];

    char inbound_endpoint[CEBUS_ENDPOINT_MAX];

    cb_thread inbound_thread;
    cb_thread outbound_thread;

    struct cb_zmq_outbound_action_entry* head;

    void *zmq_context;
} cb_zmq_transport;

cb_zmq_transport* cb_zmq_transport_new(cb_zmq_transport_configuration configuration, cb_zmq_socket_options socket_options);
void cb_zmq_transport_free(cb_zmq_transport* transport);

cb_zmq_transport_error cb_zmq_transport_start(cb_zmq_transport* transport);
cb_zmq_transport_error cb_zmq_transport_stop(cb_zmq_transport* transport);
