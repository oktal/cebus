#include "cebus/peer_descriptor.h"

#include "cebus/alloc.h"
#include "cebus/binding_key.h"
#include "cebus/subscription.h"

#include "bcl.h"

cb_peer_descriptor* cb_peer_descriptor_from_proto(cb_peer_descriptor* descriptor, const PeerDescriptor *proto)
{
    cb_peer_from_proto(&descriptor->peer, proto->peer);

    if (proto->n_subscriptions > 0)
    {
        size_t i;
        cb_array_init_with_capacity(&descriptor->subscriptions, proto->n_subscriptions, sizeof(cb_subscription));

        for (i = 0; i < proto->n_subscriptions; ++i)
        {
            cb_subscription* subscription = (cb_subscription *) cb_array_push(&descriptor->subscriptions);
            cb_subscription_from_proto(subscription, proto->subscriptions[i]);
        }
    }

    descriptor->is_persistent = cebus_bool_from_int(proto->is_persistent);
    descriptor->timestamp_utc = cb_date_time_from_proto(proto->timestamp_utc);
    descriptor->has_debugger_attached = cebus_bool_from_int(proto->has_debugger_attached);

    return descriptor;
}

cb_peer_descriptor* cb_peer_descriptor_init(cb_peer_descriptor* descriptor, const cb_peer* peer, const cb_array* subscriptions)
{
    cb_peer_copy(&descriptor->peer, peer);
    cb_array_init_with_capacity(&descriptor->subscriptions, cb_array_size(subscriptions), sizeof(cb_subscription));
    {
        cb_array_iterator iter = cb_array_iter(subscriptions);
        while (cb_array_iter_has_next(CB_ARRAY_ITER(iter)) == cebus_true)
        {
            const cb_subscription* src = (const cb_subscription *) cb_array_iter_get(iter);
            cb_subscription* dst = (cb_subscription *) cb_array_push(&descriptor->subscriptions);
            cb_subscription_copy(dst, src);
            cb_array_iter_move_next(CB_ARRAY_ITER(iter));
        }
    }
    return descriptor;
}

cb_peer_descriptor* cb_peer_descriptor_new(const cb_peer* peer, const cb_array* subscriptions)
{
    return cb_peer_descriptor_init(cb_new(cb_peer_descriptor, 1), peer, subscriptions);
}

void cb_peer_descriptor_free(cb_peer_descriptor* descriptor)
{
    cb_array_free(&descriptor->subscriptions, NULL, NULL);
}

void cb_peer_descriptor_proto_from(PeerDescriptor *message, const cb_peer_descriptor *descriptor)
{
    peer_descriptor__init(message);
    const size_t n_subscriptions = cb_array_size(&descriptor->subscriptions);
    message->peer = cb_peer_proto_new(&descriptor->peer);

    message->subscriptions = cb_new(Subscription *, n_subscriptions);
    message->n_subscriptions = n_subscriptions;

    {
        cb_array_iterator iter = cb_array_iter(&descriptor->subscriptions);
        size_t i = 0;
        while (cb_array_iter_has_next(CB_ARRAY_ITER(iter)) == cebus_true)
        {
            message->subscriptions[i++] = cb_subscription_proto_new(cb_array_iter_get(iter));
            cb_array_iter_move_next(CB_ARRAY_ITER(iter));
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
