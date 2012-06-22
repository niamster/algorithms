#ifndef _BINARY_TREE_H_
#define _BINARY_TREE_H_

#include "helpers.h"
#include "sllist.h"

struct binary_tree_node {
    struct binary_tree_node *parent;
    struct binary_tree_node *left;
    struct binary_tree_node *right;
#if defined(BINARY_TREE_AVL)
    int balance;
#elif defined(BINARY_TREE_RB)
#define BINARY_TREE_RB_RED 0
#define BINARY_TREE_RB_BLACK 1
    unsigned int color;
#elif defined(BINARY_TREE_RANDOM)
    unsigned int weight;
#elif defined(BINARY_TREE_TREAP)
    unsigned int prio;
#endif
};

struct binary_tree_root {
    struct binary_tree_node root;
    struct binary_tree_node *leftmost;
    struct binary_tree_node *rightmost;
};

#if defined(BINARY_TREE_AVL)
#define BINARY_TREE_AVL_NODE_BALANCE(node) ((node)!=BINARY_TREE_EMPTY_BRANCH?(node)->balance:0)
#elif defined(BINARY_TREE_RB)
#define BINARY_TREE_RB_NODE_COLOR(node) ((node)!=BINARY_TREE_EMPTY_BRANCH?(node)->color:BINARY_TREE_RB_BLACK)
#elif defined(BINARY_TREE_RANDOM)
#define BINARY_TREE_RANDOM_NODE_WEIGHT(node) ((node)!=BINARY_TREE_EMPTY_BRANCH?(node)->weight:0)
#define BINARY_TREE_RANDOM_NODE_WEIGHT_INCLUSIVE(node) ((node)!=BINARY_TREE_EMPTY_BRANCH?(node)->weight+1:0)
#elif defined(BINARY_TREE_TREAP)
#define BINARY_TREE_TREAP_NODE_PRIO(node) ((node)!=BINARY_TREE_EMPTY_BRANCH?(node)->prio:0)
#endif

typedef cmp_result_t (*binary_tree_cmp_cbk_t)(struct binary_tree_node *, struct binary_tree_node *);
typedef cmp_result_t (*binary_tree_key_match_cbk_t)(struct binary_tree_node *, void *);
typedef void (*binary_tree_traverse_cbk_t)(struct binary_tree_node *, void *);

#define BINARY_TREE_DIRECTION_LEFT(res) ((res) == cmp_result_less)
#define BINARY_TREE_DIRECTION_RIGHT(res) ((res) == cmp_result_greater || (res) == cmp_result_equal)
#define BINARY_TREE_EMPTY_BRANCH ((struct binary_tree_node *)NULL)

static inline void __binary_tree_init_root(struct binary_tree_node *root)
{
    root->parent = root;
    root->left = BINARY_TREE_EMPTY_BRANCH;
    root->right = root;
#if defined(BINARY_TREE_AVL)
    root->balance = -1;
#elif defined(BINARY_TREE_RB)
    root->color = -1;
#elif defined(BINARY_TREE_RANDOM)
    root->weight = -1;
#elif defined(BINARY_TREE_TREAP)
    root->prio = -1;
#endif
}

static inline void binary_tree_init_root(struct binary_tree_root *root)
{
    __binary_tree_init_root(&root->root);
    root->leftmost = root->rightmost = BINARY_TREE_EMPTY_BRANCH;
}

static inline void binary_tree_init_node(struct binary_tree_node *node)
{
    node->parent = node;
    node->left = BINARY_TREE_EMPTY_BRANCH;
    node->right = BINARY_TREE_EMPTY_BRANCH;
#if defined(BINARY_TREE_AVL)
    node->balance = 0;
#elif defined(BINARY_TREE_RB)
    node->color = BINARY_TREE_RB_RED;
#elif defined(BINARY_TREE_RANDOM)
    node->weight = 0;
#elif defined(BINARY_TREE_TREAP)
    node->prio = get_random();
#endif
}

#define binary_tree_leaf_node(node)     ((node)->left == BINARY_TREE_EMPTY_BRANCH && (node)->right == BINARY_TREE_EMPTY_BRANCH)
#define binary_tree_root_node(node)     ((node)->right == (node))
#define binary_tree_empty_root(node)    (binary_tree_root_node(node) && (node)->left == BINARY_TREE_EMPTY_BRANCH)
#define binary_tree_node(node)          (binary_tree_root_node(node)?(node)->left:(node))

void __binary_tree_add(struct binary_tree_node *root,
        struct binary_tree_node *node,
        binary_tree_cmp_cbk_t cmp);
void __binary_tree_add2(struct binary_tree_root *root,
        struct binary_tree_node *node,
        binary_tree_cmp_cbk_t cmp);

static inline void binary_tree_add(struct binary_tree_node *root, struct binary_tree_node *node,
        binary_tree_cmp_cbk_t cmp)
{
    if (binary_tree_empty_root(root)) {
        root->left = node;
        node->parent = root;
#if defined(BINARY_TREE_RB)
        node->color = BINARY_TREE_RB_BLACK;
#endif
    } else {
        __binary_tree_add(root, node, cmp);
    }
}

static inline void binary_tree_add2(struct binary_tree_root *root, struct binary_tree_node *node,
        binary_tree_cmp_cbk_t cmp)
{
    struct binary_tree_node *r = &root->root;

    if (binary_tree_empty_root(r)) {
        root->leftmost = root->rightmost = r->left = node;
        node->parent = r;
#if defined(BINARY_TREE_RB)
        node->color = BINARY_TREE_RB_BLACK;
#endif
    } else {
        __binary_tree_add2(root, node, cmp);
    }
}

void __binary_tree_detach(struct binary_tree_node *node);

static inline void binary_tree_detach(struct binary_tree_node *node)
{
    if (!binary_tree_empty_root(node)) {
        __binary_tree_detach(node);
    }
}

/* Keeps rightmost and leftmost fields up to date */
static inline void binary_tree_detach2(struct binary_tree_root *root,
        struct binary_tree_node *node)
{
    if (!binary_tree_empty_root(node)) {
        if (node == root->leftmost) {
            root->leftmost = node->parent;
        } else if (node == root->rightmost) {
            root->rightmost = node->parent;
        } else {
            struct binary_tree_node *n = node->left;
            while (n) {
                if (root->leftmost = n) {
                    root->leftmost = node->parent;
                    goto detach;
                }

                n = node->left;
            }

            n = node->right;
            while (n) {
                if (root->rightmost = n) {
                    root->rightmost = node->parent;
                    goto detach;
                }

                n = node->right;
            }

          detach:
            __binary_tree_detach(node);
        }
    }
}

void __binary_tree_remove(struct binary_tree_node *node);

static inline binary_tree_remove(struct binary_tree_node *node)
{
    if (!binary_tree_empty_root(node)) {
        __binary_tree_remove(node);
    }
}

/* Keeps rightmost and leftmost fields up to date */
static inline binary_tree_remove2(struct binary_tree_root *root,
        struct binary_tree_node *node)
{
    if (!binary_tree_empty_root(node)) {
        if (root->leftmost == node) {
            if (binary_tree_root_node(node->parent))
                root->leftmost = node->right;
            else
                root->leftmost = node->parent;
        } else if (root->rightmost == node) {
            if (binary_tree_root_node(node->parent))
                root->rightmost = node->left;
            else
                root->rightmost = node->parent;
        }

        __binary_tree_remove(node);
    }
}

struct binary_tree_node_search_result {
    struct sllist list;
    struct binary_tree_node *node;
};

void __binary_tree_search(struct binary_tree_node *root,
                        void *key,
                        binary_tree_key_match_cbk_t match,
                        struct sllist *results,
                        int limit);

/**
 * return list of binary_tree_search_result instances
 */
static inline void binary_tree_search(struct binary_tree_node *root,
        void *key,
        binary_tree_key_match_cbk_t match,
        struct sllist *results,
        int limit)
{
    if (!binary_tree_empty_root(root)) {
        __binary_tree_search(binary_tree_node(root), key, match, results, limit);
    }
}

void binary_tree_search_results_free(struct sllist *results);

typedef enum {
    binary_tree_traverse_type_infix,
    binary_tree_traverse_type_prefix,
    binary_tree_traverse_type_postfix
} binary_tree_traverse_type_t;

void __binary_tree_traverse_infix(struct binary_tree_node *root,
                                  binary_tree_traverse_cbk_t cbk,
                                  void *user_data);

void __binary_tree_traverse_prefix(struct binary_tree_node *root,
                                   binary_tree_traverse_cbk_t cbk,
                                   void *user_data);

void __binary_tree_traverse_postfix(struct binary_tree_node *root,
                                    binary_tree_traverse_cbk_t cbk,
                                    void *user_data);

static inline void binary_tree_traverse(binary_tree_traverse_type_t type,
        struct binary_tree_node *root,
        binary_tree_traverse_cbk_t cbk,
        void *user_data)
{
    if (!binary_tree_empty_root(root)) {
        switch (type) {
            case binary_tree_traverse_type_infix:
                __binary_tree_traverse_infix(binary_tree_node(root), cbk, user_data);
                break;
            case binary_tree_traverse_type_prefix:
                __binary_tree_traverse_prefix(binary_tree_node(root), cbk, user_data);
                break;
            case binary_tree_traverse_type_postfix:
                __binary_tree_traverse_postfix(binary_tree_node(root), cbk, user_data);
                break;
        }
    }
}

#endif
