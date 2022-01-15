#include "cebus/transport/zmq_transport.h"
#include "cebus/transport/zmq.h"

#include "cebus/alloc.h"
#include "cebus/log.h"
#include "cebus/peer.h"
#include "cebus/peer_id.h"

typedef void (*cb_zmq_outbound_action_callback)(void *);

typedef struct cb_zmq_outbound_action_send_info
{
    cb_zmq_transport* transport;
    cb_transport_message* message;
    cb_peer_list* peers;
} cb_zmq_outbound_action_send_info;

typedef struct cb_zmq_outbound_action_disconnect_info
{
    cb_transport_message* message;
} cb_zmq_outbound_action_disconnect_info;

typedef struct cb_zmq_outbound_action
{
    cb_zmq_outbound_action_callback callback;
    void* data;
    struct cb_zmq_outbound_action* next;
} cb_zmq_outbound_action;

static void transport_message_print(const cb_transport_message* message, FILE* out)
{
    const originator_info* originator = &message->originator;

    char uuid[36];
    cb_uuid_print(&message->id.value, uuid, sizeof(uuid));

    fprintf(out, "TransportMessage { id = %s, type_id = %s, originator = { sender_id = %s, sender_endpoint = %s, sender_machiine = %s, initiator_user_name = %s }, data = %p, size = %zu }\n",
            uuid, message->message_type_id.value, originator->sender_id.value, originator->sender_endpoint, originator->sender_machine, originator->initiator_user_name, message->data, message->n_data);
}

static cb_zmq_outbound_socket* cb_zmq_transport_get_outbound_socket(cb_zmq_transport* transport, cb_peer* peer)
{
    cb_hash_value_t* socket_value = cb_hash_get(transport->outbound_sockets, &peer->peer_id);
    cb_zmq_outbound_socket* socket;
    if (socket_value == NULL)
    {
        cb_zmq_outbound_socket_error rc;
        socket = cb_zmq_outbound_socket_new(transport->zmq_context,  &peer->peer_id, peer->endpoint, transport->socket_options);
        cb_peer_id* peer_id;

        if ((rc = cb_zmq_outbound_socket_connect(socket)) != cb_zmq_outbound_socket_ok)
        {
            CB_LOG_DBG(CB_LOG_LEVEL_ERROR, "Failed to connect to peer %s (%s): %s" , peer->peer_id.value, peer->endpoint, strerror(socket->zmq_error));
            free(socket);
            return NULL;
        }

        CB_LOG_DBG(CB_LOG_LEVEL_DEBUG, "Connected to peer %s (%s)", peer->peer_id.value, peer->endpoint);

        peer_id = cb_alloc(cb_peer_id, 1);
        cb_peer_id_set(peer_id, peer->peer_id.value);

        cb_hash_insert(transport->outbound_sockets, peer_id, socket);
    }
    else
    {
        socket = (cb_zmq_outbound_socket *)socket_value;
    }

    return socket;
}

static void cb_zmq_transport_outbound_action_send(void* data)
{
    cb_zmq_outbound_action_send_info* info = (cb_zmq_outbound_action_send_info *) data;
    cb_zmq_transport* transport = info->transport;
    cb_transport_message* message = info->message;

    cb_peer_list* target_peers = info->peers;
    cb_peer_entry* peer_entry = target_peers->head;

    TransportMessage* transport_message = cb_transport_message_proto_new(message);
    size_t transport_message_size;
    uint8_t* transport_message_buf = cb_pack_message((const ProtobufCMessage *) transport_message, &transport_message_size);

    transport_message_print(message, stdout);

    while (peer_entry != NULL)
    {
        cb_peer* peer = peer_entry->peer;
        cb_zmq_outbound_socket* outbound_socket = cb_zmq_transport_get_outbound_socket(transport, peer);

        if (outbound_socket != NULL)
        {
            cb_zmq_outbound_socket_error rc;

            CB_LOG_DBG(CB_LOG_LEVEL_TRACE, "Sending %zu bytes to %s (%s)",
                    transport_message_size, peer->peer_id.value, peer->endpoint);

            if ((rc = cb_zmq_outbound_socket_send(outbound_socket, transport_message_buf, transport_message_size)) != cb_zmq_outbound_socket_ok)
                fprintf(stderr, "Failed to send to %s [%s]: %s\n", peer->peer_id.value, peer->endpoint, zmq_strerror(outbound_socket->zmq_error));
        }

        peer_entry = peer_entry->next;
    }

    cb_transport_message_proto_free(transport_message);
    cb_peer_list_free(target_peers);
    free(transport_message_buf);
    free(data);
}

static void cb_zmq_transport_outbound_action_disconnect(void* data)
{
    cb_zmq_outbound_action_disconnect_info* info = (cb_zmq_outbound_action_disconnect_info *) data;

    free(data);
}

static void* cb_zmq_transport_create_zmq_context(const cb_zmq_socket_options* options)
{
    void* context = zmq_ctx_new();
    if (options->maximum_socket_count > 0)
        zmq_ctx_set(context, ZMQ_MAX_SOCKETS, options->maximum_socket_count);

    return context;
}

static cebus_bool cb_zmq_transport_validate_message(cb_zmq_transport* transport, const TransportMessage* message)
{
    if (strncmp(transport->environment, message->environment, CEBUS_STR_MAX) != 0)
    {
        CB_LOG_DBG(CB_LOG_LEVEL_WARN, "Received transport message from wrong environment: %s != %s",
                message->environment, transport->environment);

        return cebus_false;
    }

    return cebus_true;
}

static void cb_zmq_transport_send_end_of_stream_ack(cb_zmq_transport* transport, const TransportMessage* message)
{
}

static void cb_zmq_transport_handle_end_of_stream_ack(cb_zmq_transport* transport, const TransportMessage* message)
{
}

static void cb_zmq_transport_handle_message(cb_zmq_transport* transport, const TransportMessage* message)
{
    if (cb_zmq_transport_validate_message(transport, message) == cebus_false)
    {
        return;
    }
    else
    {
        const MessageTypeId* type_id = message->message_type_id;
        const OriginatorInfo* originator = message->originator;

        CB_LOG_DBG(CB_LOG_LEVEL_DEBUG, "Received message %s from %s (%s)",
                type_id->full_name, originator->sender_id->value, originator->sender_endpoint);

        if (!strcmp(type_id->full_name, CB_MESSAGE_TYPE_ID_END_OF_STREAM))
            cb_zmq_transport_send_end_of_stream_ack(transport, message);
        else if (!strcmp(type_id->full_name, CB_MESSAGE_TYPE_ID_END_OF_STREAM_ACK))
            cb_zmq_transport_handle_end_of_stream_ack(transport, message);
        else
        {
            if (transport->on_message != NULL)
                transport->on_message(message);
        }
    }
}

static void* cb_zmq_transport_inbound_loop(void* arg)
{
    cb_zmq_transport *transport = (cb_zmq_transport *) arg;
    cb_zmq_inbound_socket* socket = transport->socket;
    zmq_msg_t msg;
    zmq_msg_init(&msg);

    for (;;)
    {
        cb_zmq_inbound_socket_error rc = cb_zmq_inbound_socket_read(socket, &msg);

        if (rc != cb_zmq_inbound_socket_ok)
        {
            fprintf(stderr, "Failed to read: %s (%d)\n", strerror(socket->zmq_error), rc);
            //break;
        }
        else
        {
            size_t n_bytes = zmq_msg_size(&msg);
            const uint8_t* data = zmq_msg_data(&msg);
            CB_LOG_DBG(CB_LOG_LEVEL_DEBUG, "Received %zu bytes", n_bytes);
            if (n_bytes > 0)
            {
                TransportMessage* message = transport_message__unpack(NULL, n_bytes, data);
                if (message == NULL)
                {
                    CB_LOG_DBG(CB_LOG_LEVEL_WARN, "Failed to deserialize transport message (%zu bytes)", n_bytes);
                }
                else
                {
                    cb_zmq_transport_handle_message(transport, message);
                    transport_message__free_unpacked(message, NULL);
                }
            }
        }
    }

    return NULL;
}

static void* cb_zmq_transport_outbound_loop(void* arg)
{
    cb_zmq_transport *transport = (cb_zmq_transport *) arg;

    for (;;)
    {
        cb_zmq_outbound_action* node;

        cb_mutex_lock(&transport->outbound_action_mutex);
        cb_cond_wait(&transport->outbound_action_cond, &transport->outbound_action_mutex);

        node = transport->outbound_action_head;
        transport->outbound_action_head = NULL;

        cb_mutex_unlock(&transport->outbound_action_mutex);

        while (node != NULL)
        {
            cb_zmq_outbound_action* action = node;
            action->callback(action->data);
            node = action->next;
            free(action);
        }
    }

    return NULL;
}

static void cb_zmq_transport_outbound_push(cb_zmq_transport* transport, cb_zmq_outbound_action_callback callback, void* data)
{
    cb_zmq_outbound_action* action = cb_alloc(cb_zmq_outbound_action, 1);
    action->callback = callback;
    action->data = data;

    cb_mutex_lock(&transport->outbound_action_mutex);

    action->next = transport->outbound_action_head;
    transport->outbound_action_head = action;

    cb_mutex_unlock(&transport->outbound_action_mutex);
    cb_cond_signal(&transport->outbound_action_cond);
}

cb_peer_list* cb_peer_list_new()
{
    cb_peer_list *list = cb_alloc(cb_peer_list, 1);
    list->head = list->tail = NULL;

    return list;
}

void cb_peer_list_add(cb_peer_list* list, cb_peer* peer)
{
    cb_peer_entry *entry = cb_alloc(cb_peer_entry, 1);
    entry->peer = peer;
    entry->next = NULL;
    
    if (list->tail == NULL)
    {
        list->head = entry;
        list->tail = entry;
    }
    else
    {
        list->tail->next = entry;
        list->tail = entry;
    }
}

void cb_peer_list_free(cb_peer_list* list)
{
    cb_peer_entry* entry = list->head;
    while (entry != NULL)
    {
        cb_peer_entry* node = entry;
        free(entry->peer);
        entry = entry->next;
        free(node);
    }

    free(list);
}

cb_zmq_transport* cb_zmq_transport_new(
        cb_zmq_transport_configuration configuration, cb_zmq_socket_options socket_options, cb_zmq_transport_on_message on_message)
{
    cb_zmq_transport* transport = cb_alloc(cb_zmq_transport, 1);
    cb_hash_map* outbound_sockets = cb_hash_map_new(cb_peer_id_hash, cb_peer_id_hash_eq);

    transport->configuration = configuration;
    transport->socket_options = socket_options;
    transport->outbound_action_head = NULL;
    transport->outbound_sockets = outbound_sockets;
    transport->zmq_context = NULL;

    transport->outbound_action_head = NULL;

    transport->on_message = on_message;

    cb_thread_init(&transport->inbound_thread);
    cb_thread_init(&transport->outbound_thread);

    return transport;
}

void cb_zmq_transport_configure(cb_zmq_transport* transport, const cb_peer_id* peer_id, const char* environment)
{
    cb_peer_id_set(&transport->peer_id, peer_id->value);
    strncpy(transport->environment, environment, CEBUS_STR_MAX);
}

cb_zmq_transport_error cb_zmq_transport_start(cb_zmq_transport* transport)
{
    void* context = cb_zmq_transport_create_zmq_context(&transport->socket_options);
    cebus_bool threadrc;
    cb_zmq_inbound_socket_error sockrc;
    cb_zmq_transport_error rc;

    const char* endpoint = transport->configuration.inbound_endpoint;

    cb_zmq_inbound_socket* inbound_socket = cb_zmq_inbound_socket_new(context, &transport->peer_id, endpoint, transport->socket_options);

    if ((sockrc = cb_zmq_inbound_socket_bind(inbound_socket)) != cb_zmq_inbound_socket_ok)
    {
        rc = cb_zmq_transport_error_start_inbound;
        goto error;
    }

    transport->socket = inbound_socket;

    if ((threadrc = cb_thread_spawn(&transport->inbound_thread, cb_zmq_transport_inbound_loop, transport)) == cebus_false)
    {
        rc = cb_zmq_transport_error_start_inbound;
        goto error;
    }

    if ((threadrc = cb_thread_spawn(&transport->outbound_thread, cb_zmq_transport_outbound_loop, transport)) == cebus_false)
    {
        rc = cb_zmq_transport_error_start_outbound;
        goto error;
    }

    transport->outbound_action_head = NULL;
    cb_mutex_init(&transport->outbound_action_mutex);
    cb_cond_init(&transport->outbound_action_cond);

    transport->zmq_context = context;
    return cb_zmq_transport_ok;

error:
    if (inbound_socket)
        cb_zmq_inbound_socket_free(inbound_socket);

    if (cb_thread_joinable(&transport->inbound_thread))
        cb_thread_destroy(&transport->inbound_thread);

    if (cb_thread_joinable(&transport->outbound_thread))
        cb_thread_destroy(&transport->outbound_thread);

    zmq_ctx_destroy(context);
    return rc;
}

cb_zmq_transport_error cb_zmq_transport_stop(cb_zmq_transport* transport)
{
    return cb_zmq_transport_ok;
}

const char* cb_zmq_transport_inbound_endpoint(const cb_zmq_transport* transport)
{
    if (transport->socket == NULL)
        return NULL;

    return cb_zmq_inbound_socket_endpoint(transport->socket);
}

cb_zmq_transport_error cb_zmq_transport_send(
        cb_zmq_transport* transport, cb_transport_message* transport_message, cb_peer_list* peers)
{
    cb_zmq_outbound_action_send_info* info = cb_alloc(cb_zmq_outbound_action_send_info, 1);
    info->transport = transport;
    info->message = transport_message;
    info->peers = peers;

    cb_zmq_transport_outbound_push(transport, cb_zmq_transport_outbound_action_send, info);
    return cb_zmq_transport_ok;
}

void cb_zmq_transport_free(cb_zmq_transport* transport)
{
    cb_hash_map_free(transport->outbound_sockets);
    zmq_ctx_destroy(transport->zmq_context);
    free(transport);
}
