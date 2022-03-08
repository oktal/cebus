#include <stdio.h>
#include <string.h>
#include <czmq.h>

#include "cebus/alloc.h"
#include "cebus/log.h"
#include "cebus/peer.h"
#include "cebus/peer_id.h"
#include "cebus/peer_directory.h"
#include "cebus/transport_message.h"
#include "cebus/transport/zmq_transport.h"

#include "cebus/bus.h"

#include "cebus/collection/hash_map.h"

#include "peer_descriptor.pb-c.h"
#include "register_peer_command.pb-c.h"

static void cb_peer_directory_init_subscriptions(cb_array* subscriptions)
{
    cb_array_init(subscriptions, sizeof(cb_subscription));

    cb_subscription* peer_started_subscription = (cb_subscription *) cb_array_push(subscriptions);
    cb_message_type_id_set(&peer_started_subscription->message_type_id, "Abc.Zebus.Directory.PeerStarted");
    cb_binding_key_init(&peer_started_subscription->binding_key);
}

int main(int argc, const char* argv[])
{
    cb_zmq_transport_configuration zmq_configuration;
    cb_zmq_socket_options options;
    cb_bus* bus;
    cb_bus_error err;
    cb_peer self;
    cb_transport* transport;
    cb_bus_configuration bus_configuration;
    cb_peer_directory directory;
    cb_array subscriptions;

    char* directory_endpoints[4] = { NULL, NULL, NULL, NULL };

    if (argc < 5)
    {
        fprintf(stderr, "usage ./cebus [directory-endpoint] [peer_id] [endpoint] [environment]\n");
        return 0;
    }

    cb_peer_id_set(&self.peer_id, argv[2]);
    cb_peer_set_endpoint(&self, argv[3]);

    cb_zmq_socket_options_init_default(&options);
    strcpy(zmq_configuration.inbound_endpoint, argv[3]);

    transport = cb_zmq_transport_new(zmq_configuration, options);

    bus = cb_bus_create(transport);
    if ((err = cb_bus_init(bus)) != cb_bus_ok)
    {
        CB_LOG_DBG(CB_LOG_LEVEL_ERROR, "Failed to initialize bus: %d", err);
        exit(EXIT_FAILURE);
    }

    if ((err = cb_bus_configure(bus, &self.peer_id, argv[4])) != cb_bus_ok)
    {
        CB_LOG_DBG(CB_LOG_LEVEL_ERROR, "Failed to configure bus: %d", err);
        exit(EXIT_FAILURE);
    }

    if ((err = cb_bus_start(bus)) != cb_bus_ok)
    {
        CB_LOG_DBG(CB_LOG_LEVEL_ERROR, "Failed to start bus:: %d", err);
        exit(EXIT_FAILURE);
    }

    directory_endpoints[0] = (char *) argv[1];
    bus_configuration.directoryEndpoints = directory_endpoints;

    cb_peer_directory_init(&directory, bus, bus_configuration);
    cb_peer_set_endpoint(&self, cb_transport_inbound_endpoint(transport));

    CB_LOG_DBG(CB_LOG_LEVEL_INFO, "Registering to directory ...");
    cb_peer_directory_init_subscriptions(&subscriptions);
    cb_peer_directory_register(&directory, bus, &self, &subscriptions);

    getchar();

    cb_peer_directory_unregister(&directory, bus);

    cb_bus_stop(bus);
    cb_bus_free(bus);
}
