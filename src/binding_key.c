#include "binding_key.h"

#include "iter_utils.h"

#include <stdlib.h>
#include <string.h>

binding_key *binding_key_new(const char** fragments, size_t count)
{
    size_t c = 0;

    binding_key* key = malloc(sizeof *key);
    if (key == NULL)
        return NULL;

    key->proto.parts = malloc(count * sizeof(*fragments));
    if (key->proto.parts == NULL)
        goto error;

    cebus_for_range(i, 0, count, {
        key->proto.parts[i] = strdup(fragments[i]);
        if (key->proto.parts[i] == NULL)
            goto error;

        ++c;
    });
    
    key->proto.n_parts = count;
    return key;

error:
    cebus_for_range(j, 0, c, {
        free(key->proto.parts[j]);
    });
    free(key);
    return NULL;
}

size_t binding_key_fragment_count(const binding_key* key)
{
    return key->proto.n_parts;
}

binding_key_fragment binding_key_get_fragment(const binding_key* key, size_t index)
{
    binding_key_fragment fragment;
    if (index >= key->proto.n_parts)
        fragment.value = NULL;
    else
        fragment.value = key->proto.parts[index];

    return fragment;
}
