//! This file defines the interface for the underlying transport layer that the bus will user to talk
/// to other peers
#pragma once

#include "cebus/peer.h"
#include "cebus/peer_id.h"
#include "cebus/transport_message.h"

#define CB_TRANSPORT_VA_ARGS(...) , ##__VA_ARGS__
#define CB_TRANSPORT_VIRTUAL(ret, func_name, ...) \
    ret (*func_name##_func)(struct cb_transport* transport CB_TRANSPORT_VA_ARGS(__VA_ARGS__))

typedef int cb_transport_error;
#define CB_TRANSPORT_OK(err) ((err) == 0)

/// The function that gets called by the transport layer when a message has been received
typedef void (*cb_transport_on_message)(const TransportMessage* message, void* user);

/// The transport layer interface
typedef struct cb_transport
{
    CB_TRANSPORT_VIRTUAL(void, on_message_received, cb_transport_on_message on_message, void* user);

    CB_TRANSPORT_VIRTUAL(void, configure, const cb_peer_id* peer_id, const char* environment);

    CB_TRANSPORT_VIRTUAL(cb_transport_error, start);
    CB_TRANSPORT_VIRTUAL(cb_transport_error, stop);
    CB_TRANSPORT_VIRTUAL(cb_transport_error, send, cb_transport_message* transport_message, cb_peer_list* peers);

    CB_TRANSPORT_VIRTUAL(const cb_peer_id*, peer_id);
    CB_TRANSPORT_VIRTUAL(const char*, inbound_endpoint);

    CB_TRANSPORT_VIRTUAL(void, free);
} cb_transport;

/// Set the `on_message` function to call back when a message is received with the provider `user` data
void cb_transport_on_message_received(cb_transport* transport, cb_transport_on_message on_message, void* user);

/// Configure the underlying `transport` layer with the given `peer_id` and `environment`
void cb_transport_configure(cb_transport* transport, const cb_peer_id* peer_id, const char* environment);

/// Start the transport
/// Return 0 if success or non-zero status code corresponding to the specific error of the underlying layer if error
cb_transport_error cb_transport_start(cb_transport* transport);

/// Stop the transport
/// Return 0 if success or non-zero status code corresponding to the specific error of the underlying layer if error
cb_transport_error cb_transport_stop(cb_transport* transport);

/// Send a `transport_message` to a given list of target `peers`
/// Return 0 if success or non-zero status code corresponding to the specific error of the underlying layer if error
cb_transport_error cb_transport_send(cb_transport* transport, cb_transport_message* transport_message, cb_peer_list* peers);

/// Get the peer id used by the given `transport`
const cb_peer_id* cb_transport_peer_id(cb_transport* transport);

/// Get the string representation of the endpoint that has been bound by the `transport` layer
const char* cb_transport_inbound_endpoint(cb_transport* transport);

/// Free the ressources owned by the `transport` layer
void cb_transport_free(cb_transport* transport);

#undef CB_TRANSPORT_VIRTUAL
#undef CB_TRANSPORT_VA_ARGS
