#pragma once

#include "cebus/config.h"
#include "cebus/utils/timespan.h"

typedef struct cb_zmq_transport_configuration
{
    char inbound_endpoint[CEBUS_ENDPOINT_MAX];

    timespan end_of_stream_ack_timeout;
} cb_zmq_transport_configuration;
