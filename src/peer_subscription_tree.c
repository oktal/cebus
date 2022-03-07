#include "cebus/peer_subscription_tree.h"
#include "cebus/alloc.h"

#include <string.h>

typedef struct cb_peer_subscription_tree_collector
{
    cb_binding_key binding_key;

    cb_peer_list peers;
} cb_peer_subscription_tree_collector;

typedef void (*cb_peer_subscription_tree_node_update_action)(cb_peer_subscription_tree_node* node, cb_peer* peer);

static void cb_peer_subscription_tree_collector_init(cb_peer_subscription_tree_collector* collector, cb_binding_key key)
{
    collector->binding_key = key;
    cb_peer_list_init(&collector->peers);
}

static void cb_peer_list_insert(cb_peer_list* peers, cb_peer* peer)
{
    const cb_peer_id* peer_id = &peer->peer_id;
    cebus_bool updated = cebus_false;
    cb_peer_list_iterator it = cb_peer_list_iter_mut(peers);

    while (cb_peer_list_iter_has_next(it) == cebus_true)
    {
        cb_peer* current_peer = cb_peer_list_iter_peer_mut(it);
        const cb_peer_id* current_peer_id = &current_peer->peer_id;

        if (cb_peer_id_eq(current_peer_id, peer_id) == cebus_true)
        {
            cb_peer_copy(current_peer, peer);
            updated = cebus_true;
            break;
        }

        cb_peer_list_iter_move_next(&it);
    }

    if (updated == cebus_false)
        cb_peer_list_add(peers, peer);
}

static void cb_peer_subscription_tree_collector_offer(cb_peer_subscription_tree_collector* collector, cb_peer_list* peers)
{
    cb_peer_list_iterator it = cb_peer_list_iter_mut(peers);
    while (cb_peer_list_iter_has_next(it) == cebus_true)
    {
        cb_peer* peer = cb_peer_list_iter_peer_mut(it);
        cb_peer_list_insert(&collector->peers, peer);
        cb_peer_list_iter_move_next(&it);
    }
}

static void cb_peer_subscription_tree_collector_offer_all(cb_peer_subscription_tree_collector* collector, cb_peer_subscription_tree_node* node)
{
    cb_peer_subscription_tree_bucket* bucket = node->head;
    cb_peer_subscription_tree_collector_offer(collector, &node->peers);

    if (node->sharp_node != NULL)
        cb_peer_subscription_tree_collector_offer_all(collector, node->sharp_node);

    if (node->star_node != NULL)
        cb_peer_subscription_tree_collector_offer_all(collector, node->star_node);

    while (bucket != NULL)
    {
        size_t i;
        for (i = 0; i < bucket->depth; ++i)
        {
            cb_peer_subscription_tree_collector_offer_all(collector, bucket->children[i]);
        }

        bucket = bucket->next;
    }
}

static cebus_bool cb_peer_subscription_tree_bucket_full(const cb_peer_subscription_tree_bucket* bucket)
{
    return cebus_bool_from_int(bucket->depth == CB_SUBSCRIPTION_TREE_BUCKET_MAX);
}

static cb_peer_subscription_tree_bucket* cb_peer_subscription_tree_bucket_add(cb_peer_subscription_tree_node* node)
{
    cb_peer_subscription_tree_bucket* bucket = cb_new(cb_peer_subscription_tree_bucket, 1);
    size_t i;

    bucket->next = NULL;
    bucket->depth = 0;
    for (i = 0; i < CB_SUBSCRIPTION_TREE_BUCKET_MAX; ++i)
    {
        bucket->children[i] = NULL;
    }

    if (node->head == NULL)
    {
        node->head = node->tail = bucket;
    }
    else
    {
        node->tail->next = bucket;
        node->tail = bucket;
    }

    return bucket;
}

static cb_peer_subscription_tree_bucket* cb_peer_subscription_tree_get_or_add_bucket(cb_peer_subscription_tree_node* node)
{
    if (node->head == NULL || cb_peer_subscription_tree_bucket_full(node->tail))
        return cb_peer_subscription_tree_bucket_add(node);

    return node->tail;
}

static cebus_bool cb_peer_subscription_tree_is_leaf(const cb_peer_subscription_tree* tree, const cb_peer_subscription_tree_node* node, cb_binding_key binding_key)
{
    if (node == tree->root)
        return cebus_false;

    return cebus_bool_from_int(node->fragment_index == cb_binding_key_fragment_count(binding_key) - 1);
}

static void cb_peer_subscription_tree_add_or_update_peer(cb_peer_subscription_tree_node* node, cb_peer* peer)
{
    cb_peer_list_insert(&node->peers, peer);
}

static void cb_peer_subscription_tree_remove_peer(cb_peer_subscription_tree_node* node, cb_peer* peer)
{
    cb_peer_list_remove(&node->peers, peer);
}

static cb_peer_subscription_tree_node* cb_peer_subscription_tree_node_new(cb_binding_key binding_key, size_t fragment_index)
{
    cb_peer_subscription_tree_node* node = cb_new(cb_peer_subscription_tree_node, 1);
    cb_binding_key_fragment fragment = cb_binding_key_get_fragment(binding_key, fragment_index);

    node->fragment = cb_binding_key_fragment_clone(fragment);
    node->fragment_index = fragment_index;

    cb_peer_list_init(&node->peers);

    node->sharp_node = node->star_node = NULL;
    node->head = node->tail = NULL;

    return node;
}

static cb_peer_subscription_tree_node** cb_peer_subscription_tree_get_child_node(cb_peer_subscription_tree_node* parent_node, cb_binding_key binding_key, size_t fragment_index)
{
    cb_binding_key_fragment fragment = cb_binding_key_get_fragment(binding_key, fragment_index);
    size_t i;

    if (cb_binding_key_fragment_is_sharp(fragment))
        return &parent_node->sharp_node;
    if (cb_binding_key_fragment_is_star(fragment))
        return &parent_node->star_node;

    if (parent_node->head != NULL)
    {
        cb_peer_subscription_tree_bucket* bucket = parent_node->head;
        while (bucket != NULL)
        {
            for (i = 0; i < bucket->depth; ++i)
            {
                cb_peer_subscription_tree_node* child = bucket->children[i];
                fragment = cb_binding_key_get_fragment(binding_key, child->fragment_index);

                if (cb_binding_key_fragment_is_empty(fragment) == cebus_false)
                {
                    if (cb_binding_key_fragment_eq(child->fragment, fragment) == cebus_true)
                        return &bucket->children[i];
                }
            }
            bucket = bucket->next;
        }
    }

    return NULL;
}

static cb_peer_subscription_tree_node** cb_peer_subscription_tree_get_or_add_child_node(cb_peer_subscription_tree_node* parent_node, cb_binding_key binding_key, size_t fragment_index)
{
    cb_peer_subscription_tree_node** node = NULL;
    cb_binding_key_fragment fragment = cb_binding_key_get_fragment(binding_key, fragment_index);

    if (cb_binding_key_fragment_is_sharp(fragment))
    {
        node = &parent_node->sharp_node;
    }
    else if (cb_binding_key_fragment_is_star(fragment))
    {
        node = &parent_node->star_node;
    }
    else
    {
        cb_peer_subscription_tree_bucket* bucket = cb_peer_subscription_tree_get_or_add_bucket(parent_node);
        size_t i;
        for (i = 0; i < bucket->depth; ++i)
        {
            if (cb_binding_key_fragment_eq(bucket->children[i]->fragment, fragment) == cebus_true)
            {
                node = &bucket->children[i];
                break;
            }
        }

        if (node == NULL)
            node = &bucket->children[bucket->depth++];
    }

    assert(node != NULL && "node should not be null");

    if (*node == NULL)
        *node = cb_peer_subscription_tree_node_new(binding_key, fragment_index);

    return node;
}

static void cb_peer_subscription_tree_remove_bucket(cb_peer_subscription_tree_node* node, cb_peer_subscription_tree_bucket* bucket)
{
    cb_peer_subscription_tree_bucket* current_bucket = node->head;
    cb_peer_subscription_tree_bucket* prev = node->head;

    while (current_bucket != NULL)
    {
        if (current_bucket == bucket)
        {
            if (prev == node->head)
                node->head = current_bucket->next;
            else
                prev->next = current_bucket->next;

            free(current_bucket);
            break;
        }

        prev = current_bucket;
        current_bucket = current_bucket->next;
    }
}

static void cb_peer_subscription_tree_free_node(const cb_peer_subscription_tree* tree, cb_peer_subscription_tree_node* node, void* data)
{
    cb_peer_subscription_tree_bucket* bucket = node->head;
    cb_binding_key_fragment_free(&node->fragment);

    while (bucket != NULL)
    {
        free(bucket);
        bucket = bucket->next;
    }

    free(node);
}

static void cb_peer_subscription_tree_remove_node(cb_peer_subscription_tree_node* parent_node, cb_peer_subscription_tree_node** node)
{
    // The node is a "star" or "sharp" node, simply free and null it
    if (node == &parent_node->star_node || node == &parent_node->sharp_node)
    {
        cb_peer_subscription_tree_free_node(NULL, *node, NULL);
        *node = NULL;
    }
    else
    {
        // The node is a child, find its corresponding bucket
        cb_peer_subscription_tree_bucket* bucket = parent_node->head;
        while (bucket != NULL)
        {
            // Is the address of our node in the children address range ?
            const uintptr_t node_addr = (uintptr_t) node; 
            const uintptr_t start_addr = (uintptr_t) &bucket->children[0];
            const uintptr_t end_addr = (uintptr_t) &bucket->children[CB_SUBSCRIPTION_TREE_BUCKET_MAX - 1];

            if (node_addr >= start_addr && node_addr <= end_addr)
            {
                // Get the index of our node in the `children` array
                const ptrdiff_t node_index = node - &bucket->children[0];
                const size_t node_size = sizeof(*bucket->children);

                // Let's make sure we're in range
                assert(node_index < CB_SUBSCRIPTION_TREE_BUCKET_MAX);

                // Free the node
                cb_peer_subscription_tree_free_node(NULL, *node, NULL);
                *node = NULL;

                // Remove the node from the array
                memmove(bucket->children + node_index, bucket->children + node_index + 1, (bucket->depth - node_index) * node_size);

                // Update the size
                bucket->depth -= 1;

                // Bucket is empty, let's remove it
                if (bucket->depth == 0)
                    cb_peer_subscription_tree_remove_bucket(parent_node, bucket);

                break;
            }

            bucket = bucket->next;
        }
    }
}

static size_t cb_peer_subscription_tree_node_depth(const cb_peer_subscription_tree_node* node)
{
    cb_peer_subscription_tree_bucket* bucket = node->head;
    size_t depth = 0;

    if (node->sharp_node)
        depth++;

    if (node->star_node)
        depth++;

    while (bucket != NULL)
    {
        depth += bucket->depth;
        bucket = bucket->next;
    }

    return depth;
}

static size_t cb_peer_subscription_tree_node_count(const cb_peer_subscription_tree_node* node)
{
    cb_peer_subscription_tree_bucket* bucket = node->head;
    size_t count = cb_peer_list_count(&node->peers);

    if (node->sharp_node != NULL)
        count += cb_peer_subscription_tree_node_count(node->sharp_node);

    if (node->star_node != NULL)
        count += cb_peer_subscription_tree_node_count(node->star_node);

    while (bucket != NULL)
    {
        size_t i;
        for (i = 0; i < bucket->depth; ++i)
        {
            count += cb_peer_subscription_tree_node_count(bucket->children[i]);
        }

        bucket = bucket->next;
    }

    return count;
}

static void cb_peer_subscription_tree_dump_peers(const cb_peer_list* peers, FILE* output)
{
    cb_peer_list_iterator it = cb_peer_list_iter(peers);
    size_t c = 0;
    fprintf(output, " [");
    while (cb_peer_list_iter_has_next(it) == cebus_true)
    {
        const cb_peer* peer = cb_peer_list_iter_peer(it);
        if (c > 0)
            fprintf(output, ", ");

        fprintf(output, "{ %s, %s }", peer->peer_id.value, peer->endpoint);
        cb_peer_list_iter_move_next(&it);
        ++c;
    }

    fprintf(output, "]\n");
}

typedef void (*cb_peer_subscription_tree_node_visitor)(const cb_peer_subscription_tree* tree, cb_peer_subscription_tree_node* node, void* data);
static void cb_peer_subscription_tree_visit(
        const cb_peer_subscription_tree* tree,
        cb_peer_subscription_tree_node* node,
        cb_peer_subscription_tree_node_visitor visit_start,
        cb_peer_subscription_tree_node_visitor visit_end,
        void* data)
{
    cb_peer_subscription_tree_bucket* bucket = node->head;
    size_t i;

    if (visit_start)
        visit_start(tree, node, data);

    if (node->sharp_node != NULL)
    {
        cb_peer_subscription_tree_visit(tree, node->sharp_node, visit_start, visit_end, data);
    }
    if (node->star_node != NULL)
    {
        cb_peer_subscription_tree_visit(tree, node->star_node, visit_start, visit_end, data);
    }

    while (bucket != NULL)
    {
        for (i = 0; i < bucket->depth; ++i)
        {
            cb_peer_subscription_tree_visit(tree, bucket->children[i], visit_start, visit_end, data);
        }

        bucket = bucket->next;
    }

    if (visit_end)
        visit_end(tree, node, data);
}

typedef cb_peer_subscription_tree_node** (*cb_peer_subscription_tree_find_node)(cb_peer_subscription_tree_node* parent_node, cb_binding_key binding_key, size_t fragment_index);
static void cb_peer_subscription_tree_update_node(
        cb_peer_subscription_tree* tree,
        cb_peer_subscription_tree_node* node,
        cb_peer* peer,
        cb_binding_key binding_key,
        size_t fragment_index,
        cb_peer_subscription_tree_find_node get_child,
        cb_peer_subscription_tree_node_update_action action)
{
    if (cb_peer_subscription_tree_is_leaf(tree, node, binding_key))
    {
        action(node, peer);
        return;
    }
    else
    {
        cb_peer_subscription_tree_node** child = get_child(node, binding_key, fragment_index);
        size_t count = 0;

        if (child == NULL)
            return;

        cb_peer_subscription_tree_update_node(tree, *child, peer, binding_key, fragment_index + 1, get_child, action);
        count = cb_peer_subscription_tree_node_count(*child);
        if (count == 0)
            cb_peer_subscription_tree_remove_node(node, child);
    }
}

static void cb_peer_subscription_tree_collect(cb_peer_subscription_tree_collector* collector, const cb_peer_subscription_tree* tree, cb_peer_subscription_tree_node* node)
{
    cb_binding_key binding_key = collector->binding_key;
    cb_peer_subscription_tree_node** child = NULL;

    if (cb_peer_subscription_tree_is_leaf(tree, node, binding_key) == cebus_true)
    {
        cb_peer_subscription_tree_collector_offer(collector, &node->peers);
        return;
    }

    if (node->sharp_node != NULL)
        cb_peer_subscription_tree_collector_offer_all(collector, node);
    if (node->star_node != NULL)
        cb_peer_subscription_tree_collect(collector, tree, node->star_node);

    child = cb_peer_subscription_tree_get_child_node(node, binding_key, node->fragment_index);
    if (child != NULL && *child != NULL)
        cb_peer_subscription_tree_collect(collector, tree, *child);
}

typedef struct cb_peer_subscription_tree_dump_context
{
    FILE* output;
} cb_peer_subscription_tree_dump_context;

static void cb_peer_subscription_tree_dump_node(const cb_peer_subscription_tree* tree, cb_peer_subscription_tree_node* node, void* data)
{
    static const size_t Indent = 4;
    size_t i;

    cb_peer_subscription_tree_dump_context* context = (cb_peer_subscription_tree_dump_context *) data;
    FILE* output = context->output;

    for (i = 0; i < node->fragment_index * Indent; ++i)
    {
        fputc(' ', output);
    }

    if (node == tree->root)
        fprintf(output, "(root)");
    else
        fprintf(output, "%zu (%zu): %s", node->fragment_index, cb_peer_subscription_tree_node_depth(node), node->fragment.value);

    cb_peer_subscription_tree_dump_peers(&node->peers, output);
}

void cb_peer_subscription_tree_init(cb_peer_subscription_tree* tree)
{
    cb_binding_key root_key;
    cb_binding_key_init(&root_key);

    cb_peer_list_init(&tree->peers_matching_all);
    tree->root = cb_peer_subscription_tree_node_new(root_key, 0);
}

void cb_peer_subscription_tree_add(cb_peer_subscription_tree* tree, cb_peer* peer, cb_binding_key binding_key)
{
    if (cb_binding_key_is_empty(binding_key) == cebus_true)
        cb_peer_list_insert(&tree->peers_matching_all, peer);
    else
        cb_peer_subscription_tree_update_node(tree, tree->root, peer, binding_key, 0, cb_peer_subscription_tree_get_or_add_child_node, cb_peer_subscription_tree_add_or_update_peer);
}

void cb_peer_subscription_tree_remove(cb_peer_subscription_tree* tree, cb_peer* peer, cb_binding_key binding_key)
{
    if (cb_binding_key_is_empty(binding_key) == cebus_true)
        cb_peer_list_remove(&tree->peers_matching_all, peer);
    else
        cb_peer_subscription_tree_update_node(tree, tree->root, peer, binding_key, 0, cb_peer_subscription_tree_get_child_node, cb_peer_subscription_tree_remove_peer);
}

cb_peer_list cb_peer_subscription_tree_get_peers(cb_peer_subscription_tree* tree, cb_binding_key binding_key)
{
    cb_peer_subscription_tree_collector collector;
    cb_peer_subscription_tree_collector_init(&collector, binding_key);

    cb_peer_subscription_tree_collector_offer(&collector, &tree->peers_matching_all);

    if (cb_binding_key_is_empty(binding_key) == cebus_true)
    {
        cb_peer_subscription_tree_collector_offer_all(&collector, tree->root);
    }
    else
        cb_peer_subscription_tree_collect(&collector, tree, tree->root);

    return collector.peers;
}

void cb_peer_subscription_tree_dump(const cb_peer_subscription_tree* tree, FILE* output)
{
    cb_peer_subscription_tree_dump_context context = { output };
    cb_peer_subscription_tree_dump_peers(&tree->peers_matching_all, output);
    cb_peer_subscription_tree_visit(tree, tree->root, cb_peer_subscription_tree_dump_node, NULL, &context);
}

void cb_peer_subscription_tree_free(cb_peer_subscription_tree* tree)
{
    cb_peer_subscription_tree_visit(tree, tree->root, NULL, cb_peer_subscription_tree_free_node, NULL);
}
