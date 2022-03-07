#pragma once

#include "cebus/binding_key.h"
#include "binding_key.pb-c.h"

/// Initialize a `cb_binding_key` from a `BindingKey` protobuf message
void cb_binding_key_from_proto(cb_binding_key* binding_key, const BindingKey* proto);

/// Create a new `BindingKey` protobuf message from a `cb_binding_key`
BindingKey* cb_binding_key_proto_new(const cb_binding_key* binding_key);

/// Free the allocated memory by a `BindingKey` protobuf message
void cb_binding_key_proto_free(BindingKey* proto);
