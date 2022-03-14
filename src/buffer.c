#include "cebus/buffer.h"

#include "cebus/alloc.h"

#include <string.h>

cb_buffer* cb_buffer_init(cb_buffer* buffer)
{
    buffer->data = NULL;
    buffer->n_data = 0;

    return buffer;
}

cb_buffer* cb_buffer_copy(cb_buffer* dst, const cb_buffer* src)
{
    dst->data = cb_alloc(src->n_data);
    memcpy(dst->data, src->data, src->n_data);
    dst->n_data = src->n_data;

    return dst;
}

cb_buffer* cb_buffer_move(cb_buffer* dst, cb_buffer* src)
{
    dst->data = src->data;
    dst->n_data = src->n_data;

    src->data = NULL;
    src->n_data = 0;

    return dst;
}

void cb_buffer_free(cb_buffer* buffer)
{
    free(buffer->data);
    buffer->n_data = 0;
}
