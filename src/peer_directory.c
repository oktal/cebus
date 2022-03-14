#include "cebus/peer_directory.h"

#include "cebus/alloc.h"
#include "cebus/collection/array.h"
#include "cebus/cebus_bool.h"
#include "cebus/log.h"
#include "cebus/message_type_id.h"
#include "cebus/peer.h"
#include "cebus/peer_descriptor.h"

#include "bcl.h"

#include "protobuf-c/protobuf-c.h"

#include "peer_id.pb-c.h"
#include "peer_started.pb-c.h"
#include "register_peer_command.pb-c.h"
#include "register_peer_response.pb-c.h"
#include "unregister_peer_command.pb-c.h"

typedef struct cb_peer_directory_entry
{
    cb_hash_map* subscriptions_index;

    cb_peer peer;

    cebus_bool is_persistent;

} cb_peer_directory_entry;

typedef struct cb_peer_directory_iterator
{
    size_t index;

    char** const directoryEndpoints;
} cb_peer_directory_iterator;

static cb_peer_directory_iterator cb_peer_directory_iter(const cb_peer_directory* peer_directory)
{
    cb_peer_directory_iterator iter = { 0, peer_directory->configuration.directoryEndpoints };
    return iter;
}

static cebus_bool cb_peer_directory_iter_has_next(cb_peer_directory_iterator iter)
{
    return cebus_bool_from_int(iter.directoryEndpoints[iter.index] != NULL);
}

static void cb_peer_directory_iter_move_next(cb_peer_directory_iterator* iter)
{
    ++iter->index;
}

void cb_peer_directory_iter_get_peer(cb_peer_directory_iterator iter, cb_peer* peer)
{
    const char* endpoint = iter.directoryEndpoints[iter.index];
    cb_peer_set_endpoint(peer, endpoint);
    cb_peer_id_set(&peer->peer_id, "Abc.Zebus.Directory.%zu", iter.index);
}

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

static void cb_peer_directory_entry_free(cb_hash_key_t key, cb_hash_value_t value, void* user)
{
    cb_peer_directory_entry* entry = (cb_peer_directory_entry *) value;
    free(value);
}

static void cb_peer_directory_init_self_descriptor(cb_peer_directory* directory, cb_peer_descriptor* descriptor, const cb_array* subscriptions)
{
    cb_peer_descriptor_init(descriptor, &directory->self, subscriptions);
    descriptor->is_persistent = directory->configuration.is_persistent;
    descriptor->timestamp_utc = cb_date_time_utc_now();
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
            cb_peer_descriptor peer_descriptor;
            cb_peer_directory_add_or_update_entry(directory, cb_peer_descriptor_from_proto(&peer_descriptor, descriptor));
        }

        register_peer_response__free_unpacked(response, NULL);
    }

    return cb_peer_directory_ok;
}

static cb_peer_directory_error cb_peer_directory_try_register_directory(cb_peer_directory* directory, cb_bus* bus, cb_peer_descriptor* descriptor)
{
    PeerDescriptor peer_descriptor;
    RegisterPeerCommand register_command;
    cb_peer_directory_iterator directory_peer_iter = cb_peer_directory_iter(directory);
    cb_command command;

    cb_peer_descriptor_proto_from(&peer_descriptor, descriptor);
    register_peer_command__init(&register_command);
    register_command.peer = &peer_descriptor; 

    command = cb_command_from_proto(register_peer_command__command(&register_command));

    while (cb_peer_directory_iter_has_next(directory_peer_iter) == cebus_true)
    {
        cb_peer directory_peer;
        cb_command_result command_result;
        cb_peer_directory_error rc;

        cb_peer_directory_iter_get_peer(directory_peer_iter, &directory_peer);
        CB_LOG_DBG(CB_LOG_LEVEL_DEBUG, "Registering %s to directory %s (%s)", peer_descriptor.peer->id->value, directory_peer.peer_id.value, directory_peer.endpoint);

        // TODO: Handle timeout
        command_result = cb_bus_send_to(bus, CB_MOVE(command), &directory_peer);
        rc = cb_peer_directory_handle_register_peer_response(directory, command_result);
        if (rc == cb_peer_directory_ok)
            return rc;
    }

    return cb_peer_directory_registration_failed;
}

static void cb_peer_directory_handle_peer_started(ProtobufCMessage* message, void* user)
{
    cb_peer_directory* directory = (cb_peer_directory *) user;
    PeerStarted* peer_started = (PeerStarted *) message;
    PeerDescriptor* descriptor = peer_started->peer_descriptor;
    Peer* peer = descriptor->peer;

    CB_LOG_DBG(CB_LOG_LEVEL_DEBUG, "Handling PeerStarted for Peer { Id = %s, Endpoint = %s }",
            peer->id->value, peer->endpoint);

    peer_started__free_unpacked(peer_started, NULL);
}

void cb_peer_directory_init(cb_peer_directory* directory, cb_bus* bus, cb_bus_configuration configuration)
{
    cb_message_proto_invoker_init(&directory->invoker, bus);

    directory->configuration = configuration;
    directory->peers = cb_hash_map_new(cb_peer_id_hash, cb_peer_id_hash_eq);
    directory->subscriptions_index = cb_hash_map_new(cb_message_type_id_hash, cb_message_type_id_hash_eq);

    {
        cb_message_proto_invoker* invoker = &directory->invoker;
        PeerStarted peer_started = PEER_STARTED__INIT;
        cb_message_proto_invoker_add(invoker, (const ProtobufCMessage *) &peer_started, "Abc.Zebus.Directory", cb_peer_directory_handle_peer_started, directory);
    }
}

cb_peer_directory_error cb_peer_directory_register(cb_peer_directory* directory, cb_bus* bus, const cb_peer* self, const cb_array* subscriptions)
{
    cb_peer_descriptor descriptor;
    cb_peer_copy(&directory->self, self);
    directory->self.is_responding = cebus_true;
    directory->self.is_up = cebus_true;

    cb_hash_clear(directory->peers, cb_peer_directory_entry_free, NULL);

    cb_peer_directory_init_self_descriptor(directory, &descriptor, subscriptions);
    cb_peer_directory_add_or_update_entry(directory, &descriptor);

    return cb_peer_directory_try_register_directory(directory, bus, &descriptor);
}

cb_peer_directory_error cb_peer_directory_unregister(cb_peer_directory* directory, cb_bus* bus)
{
    cb_peer_directory_iterator directory_peer_iter = cb_peer_directory_iter(directory);
    UnregisterPeerCommand unregister_peer_command;
    PeerId peer_id;
    Bcl__DateTime timestamp_utc;
    cb_command command;

    unregister_peer_command__init(&unregister_peer_command);
    unregister_peer_command.peer_id = cb_peer_id_proto_init(&peer_id, &directory->self.peer_id);
    unregister_peer_command.peer_endpoint = directory->self.endpoint;
    unregister_peer_command.timestamp_utc = cb_bcl_date_time_proto_init(&timestamp_utc, cb_date_time_utc_now());
    command = cb_command_from_proto(unregister_peer_command__command(&unregister_peer_command));

    while (cb_peer_directory_iter_has_next(directory_peer_iter) == cebus_true)
    {
        cb_peer directory_peer;
        cb_command_result command_result;
        cb_peer_directory_error rc;

        cb_peer_directory_iter_get_peer(directory_peer_iter, &directory_peer);
        CB_LOG_DBG(CB_LOG_LEVEL_DEBUG, "Unregistering %s from directory %s (%s)",
                directory->self.peer_id.value, directory_peer.peer_id.value, directory_peer.endpoint);

        // TODO: Handle timeout
        command_result = cb_bus_send_to(bus, CB_MOVE(command), &directory_peer);
        if (command_result.error_code == 0)
            return cb_peer_directory_ok;
    }

    return cb_peer_directory_registration_failed;
}

