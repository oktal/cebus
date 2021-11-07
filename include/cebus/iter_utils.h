#pragma once

#define cebus_for_range(var, start, end, __VA_ARGS__) \
    do {                                              \
        size_t var;                                   \
        for (var = 0; var < end; ++var)               \
        {                                             \
            do                                        \
                __VA_ARGS__                           \
            while (0);                                \
        }                                             \
    } while(0);

