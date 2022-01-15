#pragma once

#include <czmq.h>

#define CB_ZMQ_SUCCESS 0
#define CB_ZMQ_ERROR -1

#define CB_ZMQ_IPADDR_ANY "0.0.0.0"

#define CB_ZMQ_SETSOCKOPT_TRY(socket, option_name, option)                                  \
    do {                                                                                    \
        if (zmq_setsockopt(socket, option_name, &option, sizeof(option)) != CB_ZMQ_SUCCESS) \
            return CB_ZMQ_ERROR;                                                            \
    } while (0)

