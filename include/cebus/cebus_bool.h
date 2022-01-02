#pragma once

typedef enum cebus_bool
{
    cebus_false = 0,
    cebus_true = 1
} cebus_bool;

#define cebus_bool_from_int(value) ((value) > 0 ? cebus_true : cebus_false)
