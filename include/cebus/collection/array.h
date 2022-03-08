#pragma once

#include "cebus/cebus_bool.h"

#include <stddef.h>

/// A dynamically resizing array
typedef struct cb_array
{
    /// The memory backing the underlying array
    void* data;

    /// The number of elements in the array
    size_t size;

    /// The total capacity in number of elements of the array
    size_t capacity;

    /// The size of a single array element
    size_t element_size;
} cb_array;

/// Iterator base
typedef struct cb_array_iterator_base
{
    size_t index;

    size_t size;
} cb_array_iterator_base;

/// An immutable iterator over an array
typedef struct cb_array_iterator
{
    cb_array_iterator_base base;

    const cb_array* const array;
} cb_array_iterator;

/// A mutable iteartor over an array
typedef struct cb_array_iterator_mut
{
    cb_array_iterator_base base;

    cb_array* const array;
} cb_array_iterator_mut;

#define CB_ARRAY_ITER(iter) &iter.base

/// The destructor function to call on every entry of the array
typedef void (*cb_array_dtor)(void* element, void* user);

/// Initialize a new empty array
cb_array* cb_array_init(cb_array* array, size_t element_size);

/// Initialize a new `cb_array` with `size` of elements of `element_size` memory size in bytes
cb_array* cb_array_init_with_capacity(cb_array* array, size_t size, size_t element_size);

/// Copy the content of `src` array into `dst` and call the `destructor` function with `user` provided
/// data to clear the content of the old array
cb_array* cb_array_copy(cb_array* dst, const cb_array* src, cb_array_dtor destructor, void* user);

/// Push a new element and resize the `array` if needed.
/// Return a pointer to the newly inserted element
void* cb_array_push(cb_array* array);

/// Return an immutable pointer to the element from the `array` at `index` or `NULL` if the index is out of range
const void* cb_array_get(const cb_array* array, size_t index);

/// Return a mutable pointer to the element from the `array` at `index` or `NULL` if the index is out of range
void* cb_array_get_mut(cb_array* array, size_t index);

/// Return the number of elements of the `array`
size_t cb_array_size(const cb_array* array);

/// Clear the array and call the `destructor` function with `user` provided data on every entry of the array
void cb_array_clear(cb_array* array, cb_array_dtor destructor, void* user);

/// Free the memory allocated by the `array` and call  the `destructor` function with `user` provided data
/// on every entry of the array
void cb_array_free(cb_array* array, cb_array_dtor destructor, void* user);

/// Return an immutable iterator to the beginning of the `array`
cb_array_iterator cb_array_iter(const cb_array* array);
//
/// Return a mutable iterator to the beginning of the `array`
cb_array_iterator_mut cb_array_iter_mut(cb_array* array);

/// Return whether the `cb_array_iterator` iterator reached the end of the array
cebus_bool cb_array_iter_has_next(const cb_array_iterator_base* iter);

/// Move the `cb_array_iterator` iterator to the next element
void cb_array_iter_move_next(cb_array_iterator_base* iter);

/// Return the element from the array associated to the `cb_array_iterator` iterator
const void* cb_array_iter_get(cb_array_iterator iter);

/// Return the element from the array associated to the `cb_array_iterator_mut` iterator
void* cb_array_iter_get_mut(cb_array_iterator_mut iter);

