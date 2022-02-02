#pragma once

#include "cebus/cebus_bool.h"

#include "protobuf-c/protobuf-c.h"

#include <stddef.h>

typedef struct cb_routing_member
{
    size_t index;

    size_t routing_position;

    ProtobufCFieldDescriptor* member;

} cb_routing_member;

typedef struct cb_message_descriptor
{
    ProtobufCMessageDescriptor* descriptor;

    const char* namespace;

    cebus_bool is_persistent;

    cebus_bool is_infrastructure;

} cb_message_descriptor;
