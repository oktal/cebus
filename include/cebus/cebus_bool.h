#pragma once

typedef enum cebus_bool
{
    cebus_false = 0,
    cebus_true = 1
} cebus_bool;

inline cebus_bool cebus_bool_from_int(int value)
{
    return value > 0 ? cebus_true : cebus_false;
}
