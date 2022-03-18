#pragma once

#include "cebus/bus_configuration.h"
#include "cebus/bus.h"
#include "cebus/cebus_bool.h"
#include "cebus/collection/hash_map.h"
#include "cebus/collection/array.h"
#include "cebus/dispatch/proto_message_dispatcher.h"
#include "cebus/peer.h"
#include "cebus/peer_subscription_tree.h"
#include "cebus/subscription.h"
#include "cebus/threading.h"

#include "peer_started.pb-c.h"

#include <stddef.h>

#define CB_PEER_DIRECTORY_ERROR_PEER_ALREADY_EXISTS 1001

typedef enum cb_peer_directory_error
{
    /// No error
    cb_peer_directory_ok,

    /// An error occured when trying to deserialize a message from the Peer Directory
    cb_peer_directory_deserialization_error,

    /// The peer already exists inside the Peer Directory
    cb_peer_directory_peer_already_exists,

    /// Registration failed on every endpoint tried
    cb_peer_directory_registration_failed,
} cb_peer_directory_error;

/// A client to communicate with the peer directory and store the state of the directory (peers and subscriptions) in-memory
typedef struct cb_peer_directory
{
    cb_bus_configuration configuration;

    cb_peer self;

    cb_hash_map* peers;

    cb_hash_map* subscriptions_index;

    cb_proto_message_dispatcher dispatcher;
} cb_peer_directory;

/// Initialize a new `directory` with the given bus `configuration`
void cb_peer_directory_init(cb_peer_directory* directory, cb_bus* bus, cb_bus_configuration configuration);

/// Attempt to register `self` peer to the peer `directory`.
/// Return `cb_peer_directory_ok` on success or `cb_peer_directory_error` otherwise
cb_peer_directory_error cb_peer_directory_register(cb_peer_directory* directory, cb_bus* bus, const cb_peer* self, const cb_array* subscriptions);

/// Un-register from the peer `directory`
/// Return `cb_peer_directory_ok` on success or `cb_peer_directory_error` otherwise
cb_peer_directory_error cb_peer_directory_unregister(cb_peer_directory* directory, cb_bus* bus);
