#pragma once

#include "cebus/cebus_bool.h"
#include "cebus/collection/array.h"
#include "cebus/peer.h"
#include "cebus/subscription.h"
#include "cebus/utils/time.h"

#include "peer_descriptor.pb-c.h"

#include <stdio.h>

/// Describes a `Peer` and its list of `Subscription`
typedef struct cb_peer_descriptor
{
    cb_peer peer;

    cb_array subscriptions;

    cebus_bool is_persistent;

    cb_date_time timestamp_utc;

    cebus_bool has_debugger_attached;
} cb_peer_descriptor;

/// Initialize and return a `cb_peer_descriptor` from a `PeerDescriptor` protobuf message
cb_peer_descriptor *cb_peer_descriptor_from_proto(cb_peer_descriptor* descriptor, const PeerDescriptor *proto);

/// Initialize a `cb_peer_descriptor` from `given `peer` and a list of `subscriptions`
cb_peer_descriptor* cb_peer_descriptor_init(cb_peer_descriptor* descriptor, const cb_peer* peer, const cb_array* subscriptions);

/// Create a new `cb_peer_descriptor` from a given `peer` and a list of `subscriptions`
cb_peer_descriptor* cb_peer_descriptor_new(const cb_peer* peer, const cb_array* subscriptions);

/// Free the memory allocated by `descriptor`
void cb_peer_descriptor_free(cb_peer_descriptor* descriptor);

/// Initialize a `PeerDescriptor` protobuf message from a given `cb_peer_descriptor` descriptor
void cb_peer_descriptor_proto_from(PeerDescriptor *proto, const cb_peer_descriptor *descriptor);

/// Create a new `PeerDescriptor` protobuf message from a `cb_peer_descriptor` descriptor
PeerDescriptor* cb_peer_descriptor_proto_new(const cb_peer_descriptor* descriptor);

/// Free the memory allocated by the `PeerDescriptor` protobuf message
void cb_peer_descriptor_proto_free(PeerDescriptor* proto);
