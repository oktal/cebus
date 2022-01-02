#pragma once

#include "cebus/cebus_bool.h"
#include "cebus/utils/timespan.h"

#define CB_ZMQ_SOCKET_OPTION_NONE -1

typedef struct cb_zmq_socket_options
{
    // Configures ZMQ_SNDHWM (high water mark for outbound messages).
    int send_high_watermark;

    // Configures ZMQ_SNDTIMEO (maximum time before a send operation returns).
    timespan send_timeout;

    // Number of send retries before a ZMQ socket is switched to the closed state.
    // When a ZMQ socket is in the closed state, sent messages are dropped.
    int send_retries_before_switching_to_closed_state;

    // Duration of the socket closed state after send errors.
    timespan closed_state_duration_after_send_failure;

    // Duration of the socket closed state after connection errors.
    timespan closed_state_duration_after_connect_failure;

    // Configures ZMQ_RCVHWM (high water mark for inbound messages).
    int receive_high_watermark;

    // Configures ZMQ_RCVTIMEO (maximum time before a receive operation returns).
    timespan receive_timeout;

    // When specified, configures ZMQ_MAX_SOCKETS (maximum number of ZMQ sockets).
    // When null, the default value from libzmq will be used (probably 1024).
    // Note: One ZMQ socket can generate multiple TCP sockets. This is not the maximum number of TCP sockets.
    int maximum_socket_count;

    // Configures ZMQ keepalive options (ZMQ_TCP_KEEPALIVE, ZMQ_TCP_KEEPALIVE_IDLE and ZMQ_TCP_KEEPALIVE_INTVL).
    struct keep_alive_options
    {
        cebus_bool enabled;

        timespan keep_alive_timeout;
        timespan keep_alive_interval;

    } keep_alive;
} cb_zmq_socket_options;

void cb_zmq_socket_options_init_default(cb_zmq_socket_options* options);
