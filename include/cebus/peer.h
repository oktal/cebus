#pragma once

#include "cebus/cebus_bool.h"
#include "cebus/config.h"
#include "cebus/peer_id.h"

#include "peer.pb-c.h"

typedef struct cb_peer
{
    cb_peer_id peer_id;
    char endpoint[CEBUS_ENDPOINT_MAX];

    cebus_bool is_up;
    cebus_bool is_responding;
} cb_peer;

void cb_peer_set_endpoint(cb_peer* peer, const char* value);

cb_peer* cb_peer_clone(const cb_peer* src);
void cb_peer_copy(cb_peer* dst, const cb_peer* src);

Peer* cb_peer_proto_new(const cb_peer* peer);
void cb_peer_proto_free(Peer* peer);

typedef struct cb_peer_entry
{
    cb_peer* peer;
    struct cb_peer_entry* next;
} cb_peer_entry;

/// A list of peers
typedef struct cb_peer_list
{
    cb_peer_entry* head;
    cb_peer_entry* tail;

    size_t count;
} cb_peer_list;

typedef struct cb_peer_list_iterator
{
    cb_peer_entry* entry;
} cb_peer_list_iterator;

/// Initializes a new list of peers
void cb_peer_list_init(cb_peer_list* list);

/// Create a new list of peers
cb_peer_list* cb_peer_list_new();

/// Add a `peer` to the list of peers pointed by `list`
void cb_peer_list_add(cb_peer_list* list, cb_peer* peer);

/// Remove a `peer` from the list of peers pointed by `list`
/// Return `cebus_true` if the peer has been successfully removed or `cebus_false` otherwise
cebus_bool cb_peer_list_remove(cb_peer_list* list, cb_peer* peer);

/// Return `cebus_true`if the list of peers pointed by `list` is empty or `cebus_false` otherwise
cebus_bool cb_peer_list_empty(const cb_peer_list* list);

/// Return the number of entries contained in the list of peers pointed by `list`
size_t cb_peer_list_count(const cb_peer_list* list);

/// Free the memory allocated by the list pointed by `list`
void cb_peer_list_free(cb_peer_list* list);

/// Retrieve an iterator over a list of peers
cb_peer_list_iterator cb_peer_list_iter(const cb_peer_list* list);

cebus_bool cb_peer_list_iter_has_next(cb_peer_list_iterator iter);

void cb_peer_list_iter_move_next(cb_peer_list_iterator* iter);

const cb_peer* cb_peer_list_iter_peer(cb_peer_list_iterator iter);

/// Retrieve a mutable iterator over a list of peers
cb_peer_list_iterator cb_peer_list_iter_mut(cb_peer_list* list);
cb_peer* cb_peer_list_iter_peer_mut(cb_peer_list_iterator iter);
