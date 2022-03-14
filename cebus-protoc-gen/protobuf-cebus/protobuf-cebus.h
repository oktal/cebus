#pragma once

#include "protobuf-c/protobuf-c.h"

#include <stddef.h>

typedef enum
{
    /// A routable message that has one or more routing fields
    PROTOBUF_CEBUS_MESSAGE_ROUTABLE = (1 << 0),

    /// An infrastructure message
    PROTOBUF_CEBUS_MESSAGE_INFRASTRUCTURE = (1 << 1)
} ProtobufCebusMessageFlag;

typedef enum
{
    /// A command message type
    PROTOBUF_CEBUS_MESSAGE_TYPE_COMMAND = 0,

    /// An event message type
    PROTOBUF_CEBUS_MESSAGE_TYPE_EVENT = 1,
} ProtobufCebusMessageType;

typedef struct ProtobufCebusRoutingFieldDescriptor
{
    /// The original descriptor of the protobuf field
    const ProtobufCFieldDescriptor* descriptor;

    /// The routing position of this field
    size_t routing_position;
} ProtobufCebusRoutingFieldDescriptor;

typedef struct ProtobufCebusMessageDescriptor
{
    /// The original descriptor of the protobuf message
    const ProtobufCMessageDescriptor* descriptor;

    /// The dot-separated namespace of the message
    const char* namespace_name;

    /// The type of the message
    ProtobufCebusMessageType message_type;

    /// Number of routing fields
    size_t n_routing_fields;

    /// Field descriptors of routing fields sorted by routing_position
    const ProtobufCebusRoutingFieldDescriptor* routing_fields;

    /// Zero or more `ProtobufCebusMessageFlag` flags for this message
    uint32_t flags;
} ProtobufCebusMessageDescriptor;

typedef struct ProtobufCebusCommand
{
    const ProtobufCMessage* message;

    const ProtobufCebusMessageDescriptor* descriptor;
} ProtobufCebusCommand;

typedef struct ProtobufCebusEvent
{
    const ProtobufCMessage* message;

    const ProtobufCebusMessageDescriptor* descriptor;
} ProtobufCebusEvent;
