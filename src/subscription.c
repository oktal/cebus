#include "cebus/subscription.h"

#include "cebus/alloc.h"
#include "cebus/binding_key.h"
#include "cebus/message_type_id.h"

void cb_subscription_from_proto(cb_subscription* subscription, const Subscription* proto)
{
    cb_message_type_id_from_proto(&subscription->message_type_id, proto->message_type_id);
    cb_binding_key_from_proto(&subscription->binding_key, proto->binding_key);
}

void cb_subscription_copy(cb_subscription* dst, const cb_subscription* src)
{
    cb_binding_key_copy(&dst->binding_key, &src->binding_key);
    cb_message_type_id_copy(&dst->message_type_id, &src->message_type_id);
}

cb_subscription* cb_subscription_clone(const cb_subscription* src)
{
    cb_subscription* subscription = cb_new(cb_subscription, 1);
    cb_subscription_copy(subscription, src);
    return subscription;
}

Subscription* cb_subscription_proto_new(const cb_subscription* subscription)
{
    Subscription* message = cb_new(Subscription, 1);
    subscription__init(message);

    message->message_type_id = cb_message_type_id_proto_new(&subscription->message_type_id);
    message->binding_key = cb_binding_key_proto_new(&subscription->binding_key);

    return message;
}

void cb_subscription_proto_free(Subscription* proto)
{
    cb_message_type_id_proto_free(proto->message_type_id);
    cb_binding_key_proto_free(proto->binding_key);
    free(proto);
}
