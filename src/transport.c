#include "cebus/transport/transport.h"
#include "cebus/alloc.h"

#define CB_VA_ARGS(...) , ##__VA_ARGS__
#define CB_TRANSPORT_VIRT_CALL(base, func_name, ...) \
    base->func_name##_func(base CB_VA_ARGS(__VA_ARGS__))

cb_peer_list* cb_peer_list_new()
{
    cb_peer_list *list = cb_new(cb_peer_list, 1);
    list->head = list->tail = NULL;

    return list;
}

void cb_peer_list_add(cb_peer_list* list, cb_peer* peer)
{
    cb_peer_entry *entry = cb_new(cb_peer_entry, 1);
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

void cb_transport_on_message_received(cb_transport* transport, cb_transport_on_message on_message, void* user)
{
    CB_TRANSPORT_VIRT_CALL(transport, on_message_received, on_message, user);
}

void cb_transport_configure(cb_transport* transport, const cb_peer_id* peer_id, const char* environment)
{
    return CB_TRANSPORT_VIRT_CALL(transport, configure, peer_id, environment);
}

cb_transport_error cb_transport_start(cb_transport* transport)
{
    return CB_TRANSPORT_VIRT_CALL(transport, start);
}

cb_transport_error cb_transport_stop(cb_transport* transport)
{
    return CB_TRANSPORT_VIRT_CALL(transport, stop);
}

cb_transport_error cb_transport_send(cb_transport* transport, cb_transport_message* transport_message, cb_peer_list* peers)
{
    return CB_TRANSPORT_VIRT_CALL(transport, send, transport_message, peers);
}

const cb_peer_id* cb_transport_peer_id(cb_transport* transport)
{
    return CB_TRANSPORT_VIRT_CALL(transport, peer_id);
}

const char* cb_transport_inbound_endpoint(cb_transport* transport)
{
    return CB_TRANSPORT_VIRT_CALL(transport, inbound_endpoint);
}

void cb_transport_free(cb_transport* transport)
{
    CB_TRANSPORT_VIRT_CALL(transport, free);
}
