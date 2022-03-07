#include "binding_key_proto.h"

#include "cebus/alloc.h"

void cb_binding_key_from_proto(cb_binding_key* binding_key, const BindingKey* proto)
{
    cb_binding_key_free(binding_key);
    binding_key->parts = cb_new(char *, proto->n_parts);
    binding_key->n_parts = proto->n_parts;

    {
        size_t i;
        for (i = 0; i < proto->n_parts; ++i)
        {
            binding_key->parts[i] = cb_strdup(proto->parts[i]);
        }
    }
}

BindingKey* cb_binding_key_proto_new(const cb_binding_key* binding_key)
{
    BindingKey* message = cb_new(BindingKey, 1);
    binding_key__init(message);

    message->parts = cb_new(char *, binding_key->n_parts);
    message->n_parts = binding_key->n_parts;

    {
        size_t i;
        for (i = 0; i < binding_key->n_parts; ++i)
        {
            message->parts[i] = cb_strdup(binding_key->parts[i]);
        }
    }

    return message;
}

void cb_binding_key_proto_free(BindingKey* message)
{
    size_t i;
    for (i = 0; i < message->n_parts; ++i)
    {
        free(message->parts[i]);
    }

    free(message->parts);
    free(message);
}
