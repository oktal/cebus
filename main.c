#include <stdio.h>
#include <string.h>
#include <czmq.h>

#include "cebus/alloc.h"
#include "cebus/peer.h"
#include "cebus/peer_id.h"
#include "cebus/transport_message.h"
#include "cebus/transport/zmq_outbound_socket.h"

#include "peer_descriptor.pb-c.h"
#include "register_peer_command.pb-c.h"

static uint64_t get_utc_now()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return 1000000 * tv.tv_sec + tv.tv_usec;
}

void transport_message_print(const transport_message* message, FILE* out)
{
    const originator_info* originator = &message->originator;

    char uuid[36];
    uuid_unparse(message->id.value, uuid);

    fprintf(out, "TransportMessage { id = %s, type_id = %s, originator = { sender_id = %s, sender_endpoint = %s, sender_machiine = %s, initiator_user_name = %s }, data = %p, size = %zu }",
            uuid, message->message_type_id.value, originator->sender_id.value, originator->sender_endpoint, originator->sender_machine, originator->initiator_user_name, message->data, message->n_data);
}

zmq_outbound_socket* connect_directory(void* context, const peer_id* peer_id, const char* endpoint)
{
    zmq_outbound_socket* socket = outbound_socket_new(context, peer_id, endpoint);
    zmq_outbound_socket_error err;

    if (socket == NULL)
        return NULL;

    err = outbound_socket_connect(socket);
    if (err != zmq_outbound_socket_ok)
    {
        free(socket);
        return NULL;
    }

    return socket;
}

PeerDescriptor* peer_descriptor_proto_new(const peer* peer)
{
    PeerDescriptor* descriptor = cebus_alloc(sizeof* descriptor);
    peer_descriptor__init(descriptor);

    descriptor->peer = peer_proto_new(peer);
    descriptor->has_timestamp_utc = 1;
    descriptor->timestamp_utc = get_utc_now();

    return descriptor;
}

RegisterPeerCommand* register_peer_command_new(const peer* peer)
{
    RegisterPeerCommand* command = cebus_alloc(sizeof* command);
    register_peer_command__init(command);

    command->peer = peer_descriptor_proto_new(peer);
    return command;
}

void bus_send(zmq_outbound_socket* socket, const ProtobufCMessage* message, const peer_id* peer_id, const char* endpoint, const char* environment, const char *namespace)
{
    transport_message* transport_message = to_transport_message(message, peer_id, endpoint, environment, namespace);
    TransportMessage* proto = transport_message_proto_new(transport_message);
    zmq_outbound_socket_error err;

    transport_message_print(transport_message, stdout);
    putchar('\n');

    size_t size;
    void* bytes = pack_message((const ProtobufCMessage *)proto, &size);

    err = outbound_socket_send(socket, bytes, size);
    if (err != zmq_outbound_socket_ok)
        fprintf(stderr, "Failed to send: %s", zmq_strerror(zmq_errno()));

    free(transport_message);
    transport_message_proto_free(proto);
}

peer* register_directory(zmq_outbound_socket* socket, const peer_id* peer_id, const char* endpoint, const char* environment)
{
    peer* self = cebus_alloc(sizeof* self);
    RegisterPeerCommand* register_command;

    peer_id_set(&self->id, peer_id->value);
    peer_set_endpoint(self, endpoint);

    register_command = register_peer_command_new(self);
    bus_send(socket, (const ProtobufCMessage *)register_command, peer_id, endpoint, environment, "Abc.Zebus.Directory");

    return self;
}


int main(int argc, const char* argv[])
{
    void* context;
    zmq_outbound_socket* directory_connection;
    peer_id directory_peer;
    peer_id my_peer;
    peer* self;

    if (argc < 4)
    {
        fprintf(stderr, "usage ./cebus [directory-endpoint] [endpoint] [environment]\n");
        return 0;
    }

    context = zmq_ctx_new();

    peer_id_set(&directory_peer, "Directory.Local");
    directory_connection = connect_directory(context, &directory_peer, argv[1]);
    if (directory_connection == NULL)
    {
        fprintf(stderr, "Failed to connect to directory");
        return -1;
    }

    peer_id_set(&my_peer, "Peer.Local");
    self = register_directory(directory_connection, &my_peer, argv[2], argv[3]);

    puts("Connected to directory, press a key to exit.");
    getchar();

    free(self);
    outbound_socket_close(directory_connection);
    zmq_ctx_destroy(context);

    return 0;
}
