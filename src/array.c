#include "cebus/collection/array.h"

#include "cebus/alloc.h"

#include <string.h>

static size_t cb_array_offset_n(const cb_array* array, size_t index)
{
    return index * array->element_size;
}

static size_t cb_array_grow_size(const cb_array* array, size_t target_size)
{
    size_t size = array->size;
    if (size == 0)
        size = 1;

    while (size <= target_size)
        size *= 2;

    return size;
}

static void* cb_array_grow(cb_array* array, size_t new_capacity)
{
    const size_t alloc_size = new_capacity * array->element_size;
    void* new_data = cb_alloc(alloc_size);

    memcpy(new_data, array->data, array->size * array->element_size);
    free(array->data);
    array->data = new_data;
    array->capacity = new_capacity;

    return new_data;
}

static void* cb_array_ensure(cb_array* array, size_t size)
{
    const size_t needed_size = array->size + size;
    if (needed_size >= array->capacity)
    {
        size_t new_size = cb_array_grow_size(array, needed_size);
        cb_array_grow(array, new_size);
    }

    return array->data + cb_array_offset_n(array, array->size);
}

const void* cb_array_get(const cb_array* array, size_t index)
{
    if (index >= array->capacity)
        return NULL;

    return array->data + cb_array_offset_n(array, index); 
}

void* cb_array_get_mut(cb_array* array, size_t index)
{
    if (index >= array->capacity)
        return NULL;

    return array->data + cb_array_offset_n(array, index); 
}

cb_array* cb_array_init(cb_array* array, size_t element_size)
{
    array->data = NULL;
    array->capacity = 0;
    array->size = 0;
    array->element_size = element_size;

    return array;
}

cb_array* cb_array_init_with_capacity(cb_array* array, size_t size, size_t element_size)
{
    array->data = cb_alloc(size * element_size);
    array->capacity = size;
    array->size = 0;
    array->element_size = element_size;

    return array;
}

cb_array* cb_array_copy(cb_array* dst, const cb_array* src, cb_array_dtor destructor, void* user)
{
    void* data_copy = cb_alloc(src->capacity);
    memcpy(data_copy, src->data, src->capacity);
    
    //cb_array_free(dst, destructor, user);
    dst->data = data_copy;
    dst->capacity = src->capacity;
    dst->size = src->size;
    dst->element_size = src->element_size;

    return dst;
}

void* cb_array_push(cb_array* array)
{
    void* data = cb_array_ensure(array, 1);
    array->size += 1;
    return data;
}

size_t cb_array_size(const cb_array* array)
{
    return array->size;
}

cebus_bool cb_array_empty(const cb_array* array)
{
    return cebus_bool_from_int(array->size == 0);
}

void cb_array_clear(cb_array* array, cb_array_dtor destructor, void* user)
{
    if (destructor != NULL)
    {
        cb_array_iterator_mut iter = cb_array_iter_mut(array);
        while (cb_array_iter_has_next(CB_ARRAY_ITER(iter)) == cebus_true)
        {
            destructor(cb_array_iter_get_mut(iter), user);
            cb_array_iter_move_next(CB_ARRAY_ITER(iter));
        }
    }
    array->size = 0;
}

void cb_array_free(cb_array* array, cb_array_dtor destructor, void* user)
{
    cb_array_clear(array, destructor, user);
    free(array->data);
}

cb_array_iterator cb_array_iter(const cb_array* array)
{
    const cb_array_iterator iter = { { 0, array->size }, array };
    return iter;
}

cb_array_iterator_mut cb_array_iter_mut(cb_array* array)
{
    const cb_array_iterator_mut iter = { { 0, array->size }, array };
    return iter;
}

cebus_bool cb_array_iter_has_next(const cb_array_iterator_base* iter)
{
    return iter->index < iter->size;
}

void cb_array_iter_move_next(cb_array_iterator_base* iter)
{
    iter->index += 1;
}

const void* cb_array_iter_get(cb_array_iterator iter)
{
    return cb_array_get(iter.array, iter.base.index);
}

void* cb_array_iter_get_mut(cb_array_iterator_mut iter)
{
    return cb_array_get_mut(iter.array, iter.base.index);
}
