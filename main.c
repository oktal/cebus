#include <stdio.h>
#include <string.h>
#include <czmq.h>

#include "cebus/alloc.h"
#include "cebus/peer.h"
#include "cebus/peer_id.h"
#include "cebus/transport_message.h"
#include "cebus/transport/zmq_transport.h"

#include "cebus/collection/hash_map.h"

#include "peer_descriptor.pb-c.h"
#include "register_peer_command.pb-c.h"

static uint64_t get_utc_now()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return 1000000 * tv.tv_sec + tv.tv_usec;
}

PeerDescriptor* peer_descriptor_proto_new(const cb_peer* peer)
{
    PeerDescriptor* descriptor = cebus_alloc(sizeof* descriptor);
    peer_descriptor__init(descriptor);

    descriptor->peer = cb_peer_proto_new(peer);
    descriptor->has_timestamp_utc = 1;
    descriptor->timestamp_utc = get_utc_now();

    return descriptor;
}

RegisterPeerCommand* register_peer_command_new(const cb_peer* peer)
{
    RegisterPeerCommand* command = cebus_alloc(sizeof* command);
    register_peer_command__init(command);

    command->peer = peer_descriptor_proto_new(peer);
    return command;
}

void register_directory(cb_zmq_transport* transport,  cb_peer* self, const char* endpoint, const char* environment)
{
    RegisterPeerCommand* register_command = register_peer_command_new(self);
    cb_transport_message* transport_message = cb_to_transport_message((const ProtobufCMessage *)register_command, &self->peer_id, self->endpoint, environment, "Abc.Zebus.Directory");

    cb_peer_list* directory_peers = cb_peer_list_new();
    cb_peer* directory_peer = cb_alloc(cb_peer, 1);

    cb_peer_set_endpoint(directory_peer, endpoint);
    cb_peer_id_set(&directory_peer->peer_id, "Directory.0");
    cb_peer_list_add(directory_peers, directory_peer);

    cb_zmq_transport_send(transport, transport_message, directory_peers);
}

int main(int argc, const char* argv[])
{
    void* context;
    cb_peer_id directory_peer;
    cb_peer_id my_peer;

    cb_zmq_transport_configuration configuration;
    cb_zmq_socket_options options;
    cb_zmq_transport* transport;
    cb_zmq_transport_error rc;
    cb_peer* self;

    if (argc < 4)
    {
        fprintf(stderr, "usage ./cebus [directory-endpoint] [endpoint] [environment]\n");
        return 0;
    }

    cb_peer_id_set(&my_peer, "Abc.Peer.0");

    cb_zmq_socket_options_init_default(&options);
    strcpy(configuration.inbound_endpoint, argv[2]);

    transport = cb_zmq_transport_new(configuration, options, NULL);
    cb_zmq_transport_configure(transport, &my_peer, argv[3]);
    if ((rc = cb_zmq_transport_start(transport)) != cb_zmq_transport_ok)
    {
        fprintf(stderr, "Error starting transport: %d\n", rc);
        return -1;
    }

    self = cb_alloc(cb_peer, 1);
    cb_peer_set_endpoint(self, argv[2]);
    cb_peer_id_set(&self->peer_id, my_peer.value);

    register_directory(transport, self, argv[1], argv[3]);

    getchar();

    if ((rc = cb_zmq_transport_stop(transport)) != cb_zmq_transport_ok)
    {
        fprintf(stderr, "Error starting transport: %d\n", rc);
    }

    cb_zmq_transport_free(transport);
    return 0;
}
