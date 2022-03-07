#include "cebus/peer_directory.h"

#include "cebus/alloc.h"
#include "cebus/cebus_bool.h"
#include "cebus/iter_utils.h"
#include "cebus/log.h"
#include "cebus/message_type_id.h"
#include "cebus/peer.h"
#include "cebus/peer_descriptor.h"

#include "bcl.h"

#include "register_peer_command.pb-c.h"
#include "register_peer_response.pb-c.h"

cb_peer_descriptor *cb_peer_descriptor_from_proto(const PeerDescriptor *proto)
{
    cb_peer_descriptor* descriptor = cb_new(cb_peer_descriptor, 1);
    cb_peer_from_proto(&descriptor->peer, proto->peer);

    if (proto->n_subscriptions > 0)
    {
        size_t i;
        descriptor->subscriptions =
            cb_new(cb_subscription, proto->n_subscriptions);
        descriptor->n_subscriptions = proto->n_subscriptions;

        for (i = 0; i < descriptor->n_subscriptions; ++i)
        {
            cb_subscription_from_proto(&descriptor->subscriptions[i],
                                       proto->subscriptions[i]);
        }
    }

    descriptor->is_persistent = cebus_bool_from_int(proto->is_persistent);
    descriptor->timestamp_utc = cb_date_time_from_proto(proto->timestamp_utc);
    descriptor->has_debugger_attached = cebus_bool_from_int(proto->has_debugger_attached);

    return descriptor;
}

typedef struct cb_peer_directory_entry
{
    cb_hash_map* subscriptions_index;

    cb_peer peer;

    cebus_bool is_persistent;

} cb_peer_directory_entry;

static void cb_peer_directory_print_peer_entry(const cb_hash_key_t key, const cb_hash_value_t value, void* user)
{
    const cb_peer_id* peer_id = (const cb_peer_id *) key;
    const cb_peer_directory_entry* entry = (const cb_peer_directory_entry *) value;
    const cb_peer* peer = &entry->peer;
    FILE* stream = (FILE *) user;

    fprintf(stream, "Peer { Id = %s, Endpoint = %s, IsUp = %d }\n",
            peer->peer_id.value, peer->endpoint, peer->is_up);
}

static void cb_peer_directory_print(const cb_peer_directory* directory, FILE* stream)
{
    cb_hash_map* peers = directory->peers;
    fprintf(stream, "Self { Id = %s, Endpoint = %s }\n", directory->self.peer_id.value, directory->self.endpoint);
    cb_hash_foreach(peers, cb_peer_directory_print_peer_entry, stream);
}

static cb_peer_directory_entry* cb_peer_directory_entry_new(const cb_peer_descriptor* descriptor, cb_hash_map* subscriptions_index)
{
    cb_peer_directory_entry* entry = cb_new(cb_peer_directory_entry, 1);
    entry->subscriptions_index = subscriptions_index;
    cb_peer_copy(&entry->peer, &descriptor->peer);
    entry->is_persistent = descriptor->is_persistent;

    return entry;
}

static cb_peer_descriptor* cb_peer_directory_create_self_descriptor(cb_peer_directory* directory, const cb_subscription* subscriptions, size_t n_subscriptions)
{
    cb_peer_descriptor* descriptor = cb_peer_descriptor_new(&directory->self, subscriptions, n_subscriptions);
    descriptor->is_persistent = directory->configuration.is_persistent;
    descriptor->timestamp_utc = cb_date_time_utc_now();
    return descriptor;
}

static void cb_peer_directory_add_or_update_entry(cb_peer_directory* directory, cb_peer_descriptor* descriptor)
{
    cb_peer* peer = &descriptor->peer;
    cb_peer_id* peer_id = &peer->peer_id;
    cb_hash_map* peers = directory->peers;
    cb_hash_value_t peer_directory_entry = cb_hash_get(peers, peer_id);
    if (peer_directory_entry == NULL)
    {
        CB_LOG_DBG(CB_LOG_LEVEL_TRACE, "Creating new peer entry { %s, %s }", peer_id->value, peer->endpoint);
        cb_hash_insert(peers, cb_peer_id_clone(peer_id), cb_peer_directory_entry_new(descriptor, directory->subscriptions_index));
    }
    else
    {
        cb_peer_directory_entry* entry = (cb_peer_directory_entry *) peer_directory_entry;
        cb_peer* peer_entry = &entry->peer;

        CB_LOG_DBG(CB_LOG_LEVEL_TRACE, "Updating peer entry { %s, %s }", peer_entry->peer_id.value, peer_entry->endpoint);

        cb_peer_set_endpoint(peer_entry, peer->endpoint);
        peer_entry->is_responding = peer->is_responding;
        peer_entry->is_up = peer->is_up;
        entry->is_persistent = descriptor->is_persistent;
    }
}

static cb_peer_directory_error cb_peer_directory_handle_register_peer_response(cb_peer_directory* directory, cb_command_result command_result)
{
    RegisterPeerResponse* response = NULL;

    if (command_result.error_code == CB_PEER_DIRECTORY_ERROR_PEER_ALREADY_EXISTS)
        return cb_peer_directory_peer_already_exists;

    response = register_peer_response__unpack(NULL, command_result.data.n_data, command_result.data.data);
    if (response == NULL)
    {
        CB_LOG_DBG(CB_LOG_LEVEL_WARN, "Failed to deserialize RegisterPeerResponse (%zu bytes)", command_result.data.n_data);
        return cb_peer_directory_deserialization_error;
    }
    else
    {
        int i;
        for (i = 0; i < response->n_peer_descriptors; ++i)
        {
            const PeerDescriptor* descriptor = response->peer_descriptors[i];
            cb_peer_descriptor* peer_descriptor = cb_peer_descriptor_from_proto(descriptor);
            cb_peer_directory_add_or_update_entry(directory, peer_descriptor);
        }

        register_peer_response__free_unpacked(response, NULL);
    }

    return cb_peer_directory_ok;
}

static cb_peer_directory_error cb_peer_directory_try_register_directory(cb_peer_directory* directory, cb_bus* bus, cb_peer_descriptor* descriptor)
{
    const cb_bus_configuration* configuration = &directory->configuration;
    cb_peer directory_peer;
    RegisterPeerCommand register_command;
    RegisterPeerResponse* register_response = NULL;
    PeerDescriptor peer_descriptor;
    cb_command command;
    cb_command_result command_result;

    cb_peer_set_endpoint(&directory_peer, configuration->directoryEndpoints[0]);
    cb_peer_id_set(&directory_peer.peer_id, "Abc.Zebus.Directory.0");

    cb_peer_descriptor_proto_from(&peer_descriptor, descriptor);

    register_peer_command__init(&register_command);
    register_command.peer = &peer_descriptor; 

    CB_LOG_DBG(CB_LOG_LEVEL_DEBUG, "Registering %s to directory %s (%s)", peer_descriptor.peer->id->value, directory_peer.peer_id.value, directory_peer.endpoint);

    command = CB_COMMAND(register_command, "Abc.Zebus.Directory");
    command_result = cb_bus_send_to(bus, &command, &directory_peer);
    return cb_peer_directory_handle_register_peer_response(directory, command_result);
}

void cb_peer_directory_init(cb_peer_directory* directory, cb_bus_configuration configuration)
{
    directory->configuration = configuration;
    directory->peers = cb_hash_map_new(cb_peer_id_hash, cb_peer_id_hash_eq);
    directory->subscriptions_index = cb_hash_map_new(cb_message_type_id_hash, cb_message_type_id_hash_eq);
}

cb_peer_directory_error cb_peer_directory_register(cb_peer_directory* directory, cb_bus* bus, const cb_peer* self, const cb_subscription* subscriptions, size_t n_subscriptions)
{
    cb_peer_descriptor* descriptor;
    cb_peer_copy(&directory->self, self);
    directory->self.is_responding = cebus_true;
    directory->self.is_up = cebus_true;

    descriptor = cb_peer_directory_create_self_descriptor(directory, subscriptions, n_subscriptions);
    cb_peer_directory_add_or_update_entry(directory, descriptor);

    return cb_peer_directory_try_register_directory(directory, bus, descriptor);
}

void cb_peer_directory_handle_peer_started(PeerStarted* message)
{
}
