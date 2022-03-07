#include "message_serializer.h"

#include "cebus/alloc.h"

void* cb_pack_message(const ProtobufCMessage* proto, size_t *size_out)
{
    const size_t packed_size = protobuf_c_message_get_packed_size(proto);
    if (size_out != NULL)
        *size_out = packed_size;

    uint8_t* buf = cb_new(uint8_t, packed_size);
    protobuf_c_message_pack(proto, buf);

    return buf;
}
