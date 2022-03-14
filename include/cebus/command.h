#pragma once

#include "cebus/message_type_id.h"

#include "protobuf-cebus/protobuf-cebus.h"

typedef struct cb_command
{
    void* data;

    size_t n_data;

    cb_message_type_id message_type_id;
} cb_command;

/// Initialize a new `cb_command` command
void cb_command_init(cb_command* command);

/// Copy a `cb_command` from `src` to `dst`
cb_command* cb_command_copy(cb_command* dst, const cb_command* src);

/// Move a `cb_command` from `src` to `dst`. Move operation will invalidate `src`
cb_command* cb_command_move(cb_command* dst, cb_command* src);

/// Create a `cb_command` from a `ProtobufCebusCommand` proto
cb_command cb_command_from_proto(ProtobufCebusCommand proto);

/// Free the memory allocated by `cb_command` command
void cb_command_free(cb_command* command);
