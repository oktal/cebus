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

#if 0
typedef struct sleep_context
{
    cb_future future;

    int secs;
} sleep_context;

void* sleep_thread_main(void *data)
{
    sleep_context* context = (sleep_context *)data;
    sleep(context->secs);
    cb_future_set(&context->future, NULL);

    return NULL;
}

void sleep_async(int secs)
{
    sleep_context* context = cb_new(sleep_context, 1);
    cb_thread thread;

    context->secs= secs;

    cb_future_init(&context->future);
    cb_thread_init(&thread);

    cb_thread_spawn(&thread, sleep_thread_main, context);
    cb_future_get(&context->future);

    cb_thread_join(&thread);
    cb_thread_destroy(&thread);
}

int main()
{
    printf("Sleeping 5 seconds...\n");
    sleep_async(5);
    printf("... beep beep beep\n");
}
#endif

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

    char* directory_endpoints[4] = { NULL, NULL, NULL, NULL };

    if (argc < 4)
    {
        fprintf(stderr, "usage ./cebus [directory-endpoint] [endpoint] [environment]\n");
        return 0;
    }


    cb_peer_id_set(&self.peer_id, "Abc.Peer.0");
    cb_peer_set_endpoint(&self, argv[2]);

    cb_zmq_socket_options_init_default(&options);
    strcpy(zmq_configuration.inbound_endpoint, argv[2]);

    transport = cb_zmq_transport_new(zmq_configuration, options);

    bus = cb_bus_create(transport);
    if ((err = cb_bus_init(bus)) != cb_bus_ok)
    {
        CB_LOG_DBG(CB_LOG_LEVEL_ERROR, "Failed to initialize bus: %d", err);
        exit(EXIT_FAILURE);
    }

    if ((err = cb_bus_configure(bus, &self.peer_id, argv[3])) != cb_bus_ok)
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

    cb_peer_directory_init(&directory, bus_configuration);
    cb_peer_set_endpoint(&self, cb_transport_inbound_endpoint(transport));

    CB_LOG_DBG(CB_LOG_LEVEL_INFO, "Registering to directory ...");
    cb_peer_directory_register(&directory, bus, &self, NULL, 0);

    getchar();

    cb_bus_free(bus);
}
