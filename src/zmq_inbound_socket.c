#include "cebus/transport/zmq_inbound_socket.h"

#include "cebus/alloc.h"
#include "cebus/log.h"

#include <string.h>
#include <unistd.h>

static const char* cb_env_hostname()
{
    return "octal-laptop";
}

static int cb_zmq_inbound_socket_set_options(zsock_t* sock, const cb_zmq_socket_options* options)
{
    const int recv_timeout_ms       = (int) timespan_as_millis(options->receive_timeout);

    CB_ZMQ_SETSOCKOPT_TRY(sock, ZMQ_RCVHWM  , options->receive_high_watermark);
    CB_ZMQ_SETSOCKOPT_TRY(sock, ZMQ_RCVTIMEO, recv_timeout_ms);

    return 0;
}

static cb_zmq_inbound_socket_error cb_zmq_inbound_socket_get_zmq_endpoint(cb_zmq_inbound_socket* socket)
{
    size_t endpoint_opt_len = sizeof(socket->zmq_endpoint);
    const size_t endpoint_len = endpoint_opt_len;
    char* zmq_endpoint = socket->zmq_endpoint;
    int rc = zmq_getsockopt(socket->sock, ZMQ_LAST_ENDPOINT, zmq_endpoint, &endpoint_opt_len);

    if (rc != CB_ZMQ_SUCCESS)
    {
        return cb_zmq_inbound_socket_error_sock_options;
    }
    else
    {
        const char *s = strstr(socket->zmq_endpoint, CB_ZMQ_IPADDR_ANY);
        if (s != NULL)
        {
            char sanitized_endpoint[CEBUS_ENDPOINT_MAX];
            const char *p = zmq_endpoint;
            const char* end = zmq_endpoint + endpoint_len;
            const char* hostname = cb_env_hostname();
            const size_t hostname_len = strlen(hostname);
            size_t i = 0;
            while (p < end && *p != 0 && i < CEBUS_ENDPOINT_MAX)
            {
                if (p != s)
                {
                    sanitized_endpoint[i++] = *p++;
                }
                else
                {
                    const size_t remaining = CEBUS_ENDPOINT_MAX - i;
                    const size_t n = sizeof(CB_ZMQ_IPADDR_ANY) - 1;

                    strncpy(&sanitized_endpoint[i], hostname, remaining);

                    i += hostname_len;
                    p += n;
                }
            }

            if (i >= CEBUS_ENDPOINT_MAX)
                return cb_zmq_inbound_socket_error_endpoint_too_large;

            memset(zmq_endpoint, 0, endpoint_len);
            strncpy(zmq_endpoint, sanitized_endpoint, endpoint_len);
        }

    }

    return cb_zmq_inbound_socket_ok;
}

cb_zmq_inbound_socket* cb_zmq_inbound_socket_new(
    void* context, const cb_peer_id* peer_id, const char* endpoint, cb_zmq_socket_options options)
{
    cb_zmq_inbound_socket* socket = cb_alloc(cb_zmq_inbound_socket, 1);

    socket->context   = context;
    socket->sock      = NULL;
    socket->zmq_error = 0;

    cb_peer_id_set(&socket->peer_id, peer_id->value);
    strncpy(socket->endpoint, endpoint, CEBUS_ENDPOINT_MAX);

    memset(socket->zmq_endpoint, 0, sizeof(socket->zmq_endpoint));

    socket->options = options;
    return socket;
}

cb_zmq_inbound_socket_error cb_zmq_inbound_socket_bind(cb_zmq_inbound_socket* socket)
{
    int rc;
    cb_zmq_inbound_socket_error err;

    if (socket->sock == NULL)
    {
        zsock_t* sock = zmq_socket(socket->context, ZMQ_PULL);
        if (sock == NULL)
            return cb_zmq_inbound_socket_error_sock_create;

        if ((rc = cb_zmq_inbound_socket_set_options(sock, &socket->options)) != CB_ZMQ_SUCCESS)
        {
            socket->zmq_error = errno;
            return cb_zmq_inbound_socket_error_sock_options;
        }

        socket->sock = sock;
    }

    CB_LOG_DBG(CB_LOG_LEVEL_INFO, "Binding to %s ...", socket->endpoint);
    if ((rc = zmq_bind(socket->sock, socket->endpoint)) != CB_ZMQ_SUCCESS)
    {
        socket->zmq_error = errno;
        return cb_zmq_inbound_socket_error_bind;
    }

    if ((err = cb_zmq_inbound_socket_get_zmq_endpoint(socket)) != cb_zmq_inbound_socket_ok)
    {
        socket->zmq_error = errno;
        return err;
    }

    CB_LOG_DBG(CB_LOG_LEVEL_INFO, "... bound to %s", socket->zmq_endpoint);
    return cb_zmq_inbound_socket_ok;
}

const char* cb_zmq_inbound_socket_endpoint(const cb_zmq_inbound_socket* socket)
{
    return socket->zmq_endpoint;
}

cb_zmq_inbound_socket_error cb_zmq_inbound_socket_close(cb_zmq_inbound_socket* socket)
{
    int rc;

    if (socket->sock == NULL)
        return cb_zmq_inbound_socket_error_invalid;

    if ((rc = zmq_unbind(socket->sock, socket->endpoint)) != CB_ZMQ_SUCCESS)
    {
        socket->zmq_error = errno;
        return cb_zmq_inbound_socket_error_unbind;
    }

    return cb_zmq_inbound_socket_ok;
}

cb_zmq_inbound_socket_error cb_zmq_inbound_socket_read(cb_zmq_inbound_socket* socket, zmq_msg_t* msg)
{
    int rc = 0;
    if (socket->sock == NULL)
        return cb_zmq_inbound_socket_error_invalid;

    while ((rc = zmq_recvmsg(socket->sock, msg, 0)) == CB_ZMQ_ERROR)
    {
        if (errno == EINTR)
            continue;

        socket->zmq_error = errno;
        return cb_zmq_inbound_socket_error_recv;
    }

    return cb_zmq_inbound_socket_ok;
}

void cb_zmq_inbound_socket_free(cb_zmq_inbound_socket* socket)
{
    if (socket->sock != NULL)
        cb_zmq_inbound_socket_close(socket);

    free(socket);
}
