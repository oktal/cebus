#pragma once

#include "protobuf-c/protobuf-c.h"

void* cb_pack_message(const ProtobufCMessage* proto, size_t *size_out);
