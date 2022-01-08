#pragma once

#include <czmq.h>

#define CB_ZMQ_SETSOCKOPT_TRY(socket, option_name, option)                    \
    do {                                                                      \
        if (zmq_setsockopt(socket, option_name, &option, sizeof(option)) < 0) \
            return -1;                                                        \
    } while (0)
