#pragma once

#include "cebus/binding_key.h"
#include "cebus/message_type_id.h"

#include "subscription.pb-c.h"

/// Represents a subscription over a `message_type_id` with a given `binding_key`
typedef struct cb_subscription
{
    cb_message_type_id message_type_id;

    cb_binding_key binding_key;
} cb_subscription;

/// Initialize a `cb_subscription` from a `Subscription` protobuf message
void cb_subscription_from_proto(cb_subscription* subscription, const Subscription* proto);

/// Copy a `cb_subscription` from `src` to `dst`
void cb_subscription_copy(cb_subscription* dst, const cb_subscription* src);

/// Create a new copy of a `cb_subscription` from `src`
cb_subscription* cb_subscription_clone(const cb_subscription* src);

/// Create a new `Subscription` protobuf message for a `cb_subscription`
Subscription*  cb_subscription_proto_new(const cb_subscription* subscription);

/// Free the allocated memory from a `Subscription` protobuf message
void cb_subscription_proto_free(Subscription* proto);
