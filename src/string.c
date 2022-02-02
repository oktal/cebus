#include "cebus/string.h"
#include "cebus/cebus_bool.h"

#include <string.h>
#include <stdio.h>

static cebus_bool cb_copy_str_slice(const char* str, size_t from, size_t to, char* out, size_t size)
{
    const size_t len = to - from;
    size_t i;

    if (len > size)
        return cebus_false;

    for (i = from; i < to; ++i)
    {
        *out++ = str[i];
    }

    return cebus_true;
}

void cb_str_replace(const char* src, const char* needle, const char* str, char* out, size_t size)
{
    // First, search for `needle` inside `src`
    const char* p = strstr(src, needle);
    const char* cursor = src;

    // The total size we have available (-1 for NUL-terminated)
    const size_t s = size - 1;

    size_t n = 0;

    // `needle` was not found, just copy `src` into `out`
    if (p == NULL)
    {
        strncpy(out, src, size);
        return;
    }

    for (;;)
    {
        if (*cursor == 0 || n >= s)
            break;

        if (cursor == p)
        {
            for (;;)
            {
                if (*str == 0 || n >= s)
                    break;

                *out++ = *str++;
                n += 1;
            }

            cursor += strlen(needle);
        }

        else
        {
            *out++ = *cursor++;
        }

        n += 1;
    }

    if (n < size)
        *out = 0;
}

