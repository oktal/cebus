#pragma once

#include <stdio.h>
#include <stdlib.h>

#define cebus_unimplemented()                                       \
    do {                                                            \
        fprintf(stderr, "Unimplemented %s:%d", __FILE__, __LINE__); \
        abort();                                                    \
    } while (0)
