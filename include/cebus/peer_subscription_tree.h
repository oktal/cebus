#pragma once

#include "cebus/binding_key.h"
#include "cebus/message_type_id.h"
#include "cebus/peer.h"

#include <stdio.h>

#define CB_SUBSCRIPTION_TREE_BUCKET_MAX 32

struct cb_peer_subscription_tree_bucket;
typedef struct cb_peer_subscription_tree_node
{
    cb_binding_key_fragment fragment;

    size_t fragment_index;

    cb_peer_list peers;

    struct cb_peer_subscription_tree_node* sharp_node;
    struct cb_peer_subscription_tree_node* star_node;

    struct cb_peer_subscription_tree_bucket* head;
    struct  cb_peer_subscription_tree_bucket* tail;

} cb_peer_subscription_tree_node;

typedef struct cb_peer_subscription_tree_bucket
{
    cb_peer_subscription_tree_node* children[CB_SUBSCRIPTION_TREE_BUCKET_MAX];

    size_t depth;

    struct cb_peer_subscription_tree_bucket* next;
} cb_peer_subscription_tree_bucket;

typedef struct cb_peer_subscription_tree
{
    cb_peer_subscription_tree_node* root;

    cb_peer_list peers_matching_all;
} cb_peer_subscription_tree;

void cb_peer_subscription_tree_init(cb_peer_subscription_tree* tree);

void cb_peer_subscription_tree_add(cb_peer_subscription_tree* tree, cb_peer* peer, cb_binding_key binding_key);
void cb_peer_subscription_tree_remove(cb_peer_subscription_tree* tree, cb_peer* peer, cb_binding_key binding_key);

cb_peer_list cb_peer_subscription_tree_get_peers(cb_peer_subscription_tree* tree, cb_binding_key binding_key);

void cb_peer_subscription_tree_dump(const cb_peer_subscription_tree* tree, FILE* output);
void cb_peer_subscription_tree_free(cb_peer_subscription_tree* tree);
