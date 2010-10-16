#ifndef _BINARY_TREE_H_
#define _BINARY_TREE_H_

#include "helpers.h"
#include "sllist.h"

struct binary_tree {
    struct binary_tree *parent;
    struct binary_tree *left;
    struct binary_tree *right;
    unsigned int weight;
#if defined(BINARY_TREE_AVL)
    int balance;
#elif defined(BINARY_TREE_RB)
#define BINARY_TREE_RB_RED 0
#define BINARY_TREE_RB_BLACK 1
    unsigned int color;
#endif
};

#if defined(BINARY_TREE_RB)
#define BINARY_TREE_RB_NODE_COLOR(node) ((node)!=BINARY_TREE_EMPTY_BRANCH?(node)->color:BINARY_TREE_RB_BLACK)
#endif

typedef cmp_result_t (*binary_tree_cmp_cbk_t)(struct binary_tree *, struct binary_tree *);
typedef cmp_result_t (*binary_tree_key_match_cbk_t)(struct binary_tree *, void *);
typedef void (*binary_tree_traverse_cbk_t)(struct binary_tree *, void *);

#define BINARY_TREE_DIRECTION_LEFT(res) ((res) == cmp_result_less)
#define BINARY_TREE_DIRECTION_RIGHT(res) ((res) == cmp_result_greater || (res) == cmp_result_equal)
#define BINARY_TREE_EMPTY_BRANCH ((struct binary_tree *)NULL)

#if defined(BINARY_TREE_AVL)
#define binary_tree_init_root(root)             \
    do {                                        \
        (root)->parent = (root);                \
        (root)->left = (root);                  \
        (root)->right = (root);                 \
        (root)->weight = -1;                    \
        (root)->balance = -1;                   \
    } while (0)
#elif defined(BINARY_TREE_RB)
#define binary_tree_init_root(root)             \
    do {                                        \
        (root)->parent = (root);                \
        (root)->left = (root);                  \
        (root)->right = (root);                 \
        (root)->weight = -1;                    \
        (root)->color = -1;                     \
    } while (0)
#else
#define binary_tree_init_root(root)             \
    do {                                        \
        (root)->parent = (root);                \
        (root)->left = (root);                  \
        (root)->right = (root);                 \
        (root)->weight = -1;                    \
    } while (0)
#endif

#if defined(BINARY_TREE_AVL)
#define binary_tree_init_node(node)                 \
    do {                                            \
        (node)->parent = (node);                    \
        (node)->left = BINARY_TREE_EMPTY_BRANCH;    \
        (node)->right = BINARY_TREE_EMPTY_BRANCH;   \
        (node)->weight = 0;                         \
        (node)->balance = 0;                        \
    } while (0)
#elif defined(BINARY_TREE_RB)
#define binary_tree_init_node(node)                 \
    do {                                            \
        (node)->parent = (node);                    \
        (node)->left = BINARY_TREE_EMPTY_BRANCH;    \
        (node)->right = BINARY_TREE_EMPTY_BRANCH;   \
        (node)->weight = 0;                         \
        (node)->color = BINARY_TREE_RB_RED;         \
    } while (0)
#else
#define binary_tree_init_node(node)                 \
    do {                                            \
        (node)->parent = (node);                    \
        (node)->left = BINARY_TREE_EMPTY_BRANCH;    \
        (node)->right = BINARY_TREE_EMPTY_BRANCH;   \
        (node)->weight = 0;                         \
    } while (0)
#endif

#define binary_tree_root_node(node) ((node)->left == (node)->right && (node)->left != BINARY_TREE_EMPTY_BRANCH)
#define binary_tree_top(node) ((node) == (node)->parent)
#define binary_tree_leaf_node(node) ((node)->left == BINARY_TREE_EMPTY_BRANCH && (node)->right == BINARY_TREE_EMPTY_BRANCH)
#define binary_tree_empty_root(root) ((root)->left == (root) && (root)->right == (root))
#define binary_tree_node(root) (binary_tree_root_node(root)?(root)->left:(root))

#if defined(BINARY_TREE_RB)
#define binary_tree_add(root, node, cmp)                            \
    do {                                                            \
        if (binary_tree_empty_root(root)) {                         \
            (root)->left = (root)->right = (node);                  \
            (node)->parent = (root);                                \
            (node)->color = BINARY_TREE_RB_BLACK;                   \
        } else {                                                    \
            __binary_tree_add((root)->left, node, cmp);             \
        }                                                           \
    } while (0)
#else
#define binary_tree_add(root, node, cmp)                            \
    do {                                                            \
        if (binary_tree_empty_root(root)) {                         \
            (root)->left = (root)->right = (node);                  \
            (node)->parent = (root);                                \
        } else {                                                    \
            __binary_tree_add((root)->left, node, cmp);             \
        }                                                           \
    } while (0)
#endif

#define binary_tree_detach(node)                                \
    do {                                                        \
        if (!binary_tree_empty_root(node)) {                    \
            __binary_tree_detach(node);                         \
        }                                                       \
    } while (0)

#define binary_tree_remove(node)                                \
    do {                                                        \
        if (!binary_tree_empty_root(node)) {                    \
            __binary_tree_remove(node);                         \
        }                                                       \
    } while (0)

struct binary_tree_search_result {
    struct sllist list;
    struct binary_tree *node;
};

/**
 * return list of binary_tree_search_result instances
 */
#define binary_tree_search(root, key, match, results, limit)        \
    do {                                                            \
        if (!binary_tree_empty_root(root)) {                        \
            __binary_tree_search(binary_tree_node(root), key, match, results, limit); \
        }                                                           \
    } while (0)

void binary_tree_search_results_free(struct sllist *results);

typedef enum {
    binary_tree_traverse_type_infix,
    binary_tree_traverse_type_prefix,
    binary_tree_traverse_type_postfix
} binary_tree_traverse_type_t;

#define binary_tree_traverse(type, root, cbk, user_data)                \
    do {                                                                \
        if (!binary_tree_empty_root(root)) {                            \
            switch (type) {                                             \
                case binary_tree_traverse_type_infix:                   \
                    __binary_tree_traverse_infix(binary_tree_node(root), cbk, user_data); \
                    break;                                              \
                case binary_tree_traverse_type_prefix:                  \
                    __binary_tree_traverse_prefix(binary_tree_node(root), cbk, user_data); \
                    break;                                              \
                case binary_tree_traverse_type_postfix:                 \
                    __binary_tree_traverse_postfix(binary_tree_node(root), cbk, user_data); \
                    break;                                              \
            }                                                           \
        }                                                               \
    } while (0)

void __binary_tree_traverse_infix(struct binary_tree *root,
                                  binary_tree_traverse_cbk_t cbk,
                                  void *user_data);

void __binary_tree_traverse_prefix(struct binary_tree *root,
                                   binary_tree_traverse_cbk_t cbk,
                                   void *user_data);

void __binary_tree_traverse_postfix(struct binary_tree *root,
                                    binary_tree_traverse_cbk_t cbk,
                                    void *user_data);

void __binary_tree_search(struct binary_tree *root,
                        void *key,
                        binary_tree_key_match_cbk_t match,
                        struct sllist *results,
                        int limit);

void __binary_tree_add(struct binary_tree *root,
                       struct binary_tree *node,
                       binary_tree_cmp_cbk_t cmp);

void __binary_tree_detach(struct binary_tree *node);

void __binary_tree_remove(struct binary_tree *node);

#endif
