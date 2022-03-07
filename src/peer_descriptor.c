#include "cebus/peer_descriptor.h"

#include "cebus/alloc.h"
#include "cebus/subscription.h"

#include "bcl.h"

cb_peer_descriptor* cb_peer_descriptor_new(const cb_peer* peer, const cb_subscription* subscriptions, size_t n_subscriptions)
{
    cb_peer_descriptor* descriptor = cb_new(cb_peer_descriptor, 1);
    cb_peer_copy(&descriptor->peer, peer);

    descriptor->subscriptions = cb_new(cb_subscription, n_subscriptions);
    descriptor->n_subscriptions = n_subscriptions;

    {
        size_t i;
        for (i = 0; i < n_subscriptions; ++i)
        {
            cb_subscription_copy(&descriptor->subscriptions[i], &subscriptions[i]);
        }
    }

    return descriptor;
}

void cb_peer_descriptor_free(cb_peer_descriptor* descriptor)
{
    free(descriptor->subscriptions);
    free(descriptor);
}

void cb_peer_descriptor_proto_from(PeerDescriptor *message, const cb_peer_descriptor *descriptor)
{
    peer_descriptor__init(message);
    message->peer = cb_peer_proto_new(&descriptor->peer);

    message->subscriptions = cb_new(Subscription *, descriptor->n_subscriptions);
    message->n_subscriptions = descriptor->n_subscriptions;

    {
        size_t i;
        for (i = 0; i < descriptor->n_subscriptions; ++i)
        {
            message->subscriptions[i] = cb_subscription_proto_new(&descriptor->subscriptions[i]);
        }
    }

    message->is_persistent = descriptor->is_persistent;
    message->timestamp_utc = cb_bcl_date_time_proto_new(descriptor->timestamp_utc);

    message->has_has_debugger_attached = 1;
    message->has_debugger_attached = 0;
}

PeerDescriptor* cb_peer_descriptor_proto_new(const cb_peer_descriptor* descriptor)
{
    PeerDescriptor* message = cb_new(PeerDescriptor, 1);
    cb_peer_descriptor_proto_from(message, descriptor);
    return message;
}

void cb_peer_descriptor_proto_free(PeerDescriptor* message)
{
    size_t i;
    cb_peer_proto_free(message->peer);
    for (i = 0; i < message->n_subscriptions; ++i)
    {
        cb_subscription_proto_free(message->subscriptions[i]);
    }
    free(message);
}
