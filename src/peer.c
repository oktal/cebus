#include "cebus/peer.h"

#include "cebus/alloc.h"
#include "cebus/peer_id.h"

#include <stdlib.h>
#include <string.h>

void cb_peer_from_proto(cb_peer* peer, const Peer* proto)
{
    cb_peer_id_set(&peer->peer_id, proto->id->value);
    cb_peer_set_endpoint(peer, proto->endpoint);

    peer->is_responding = cebus_bool_from_int(proto->is_responding);
    peer->is_up = cebus_bool_from_int(proto->is_up);
}

void cb_peer_set_endpoint(cb_peer* peer, const char* value)
{
    if (peer == NULL)
        return;

    strncpy(peer->endpoint, value, CEBUS_ENDPOINT_MAX);
}

Peer* cb_peer_proto_new(const cb_peer* peer)
{
    Peer* proto = cb_new(Peer, 1);
    peer__init(proto);

    proto->id = cb_peer_id_proto_new(&peer->peer_id);
    proto->endpoint = cb_strdup(peer->endpoint);
    proto->is_responding = peer->is_responding;
    proto->is_up = peer->is_up;

    return proto;
}

void cb_peer_proto_free(Peer* peer)
{
    if (peer == NULL)
        return;

    free(peer->id);
    free(peer->endpoint);
    free(peer);
}

void cb_peer_copy(cb_peer* dst, const cb_peer* src)
{
    cb_peer_id_copy(&dst->peer_id, &src->peer_id);
    cb_peer_set_endpoint(dst, src->endpoint);
    dst->is_up = src->is_up;
    dst->is_responding = src->is_responding;
}

cb_peer* cb_peer_clone(const cb_peer* src)
{
    cb_peer* peer = cb_new(cb_peer, 1);
    cb_peer_copy(peer, src);
    return peer;
}

void cb_peer_list_init(cb_peer_list* list)
{
    list->head = list->tail = NULL;
}

cb_peer_list* cb_peer_list_new()
{
    cb_peer_list *list = cb_new(cb_peer_list, 1);
    cb_peer_list_init(list);
    return list;
}

void cb_peer_list_add(cb_peer_list* list, cb_peer* peer)
{
    cb_peer_entry *entry = cb_new(cb_peer_entry, 1);
    entry->peer = cb_peer_clone(peer);
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

    list->count += 1;
}

cebus_bool cb_peer_list_remove(cb_peer_list* list, cb_peer* peer)
{
    const cb_peer_id* peer_id = &peer->peer_id;
    cb_peer_entry* entry = list->head;
    cb_peer_entry* prev = list->head;

    while (entry != NULL)
    {
        cb_peer* current_peer = entry->peer;
        const cb_peer_id* current_peer_id = &current_peer->peer_id;

        if (cb_peer_id_eq(current_peer_id, peer_id) == cebus_true)
        {
            cb_peer_entry* tmp = entry;

            if (prev == list->head)
                list->head = list->head->next;
            else
                prev->next = entry->next;

            entry = tmp->next;

            free(current_peer);
            free(tmp);
            list->count -= 1;
            return cebus_true;
        }

        prev = entry;
        entry = entry->next;
    }

    return cebus_false;
}

cebus_bool cb_peer_list_empty(const cb_peer_list* list)
{
    return cebus_bool_from_int(list->count == 0);
}

size_t cb_peer_list_count(const cb_peer_list* list)
{
    return list->count;
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
}

cb_peer_list_iterator cb_peer_list_iter(const cb_peer_list* list)
{
    cb_peer_list_iterator iter;
    iter.entry = list->head;

    return iter;
}

cb_peer_list_iterator cb_peer_list_iter_mut(cb_peer_list* list)
{
    cb_peer_list_iterator iter;
    iter.entry = list->head;

    return iter;
}

cebus_bool cb_peer_list_iter_has_next(cb_peer_list_iterator iter)
{
    return cebus_bool_from_int(iter.entry != NULL);
}

void cb_peer_list_iter_move_next(cb_peer_list_iterator* iter)
{
    if (iter->entry != NULL)
        iter->entry = iter->entry->next;
}

const cb_peer* cb_peer_list_iter_peer(cb_peer_list_iterator iter)
{
    if (iter.entry == NULL)
        return NULL;

    return iter.entry->peer;
}

cb_peer* cb_peer_list_iter_peer_mut(cb_peer_list_iterator iter)
{
    if (iter.entry == NULL)
        return NULL;

    return iter.entry->peer;
}
