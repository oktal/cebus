#include "cebus/transport/zmq_socket_options.h"

void cb_zmq_socket_options_init_default(cb_zmq_socket_options* options)
{
    options->send_high_watermark = 20000;
    options->send_timeout = timespan_from_millis(1100);
    options->send_retries_before_switching_to_closed_state = 2;

    options->closed_state_duration_after_send_failure = timespan_from_secs(15);
    options->closed_state_duration_after_connect_failure = timespan_from_minutes(15);

    options->receive_high_watermark = 40000;
    options->receive_timeout =  timespan_from_secs(30);

    options->maximum_socket_count = 2048;

    options->keep_alive.enabled = cebus_false;
    options->keep_alive.keep_alive_timeout = timespan_from_secs(30);
    options->keep_alive.keep_alive_timeout = timespan_from_secs(2);
}
