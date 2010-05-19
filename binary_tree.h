#ifndef _BINARY_TREE_H_
#define _BINARY_TREE_H_

#include "helpers.h"
#include "sllist.h"

struct binary_tree {
    struct binary_tree *parent;
    struct binary_tree *left;
    struct binary_tree *right;
    unsigned int weight;
};

typedef cmp_result_t (*binary_tree_cmp_cbk_t)(struct binary_tree *, struct binary_tree *);
typedef cmp_result_t (*binary_tree_key_match_cbk_t)(struct binary_tree *, void *);
typedef void (*binary_tree_traverse_cbk_t)(struct binary_tree *, void *);

#define BINARY_TREE_DIRECTION_LEFT(res) ((res) == cmp_result_less)
#define BINARY_TREE_DIRECTION_RIGHT(res) ((res) == cmp_result_greater || (res) == cmp_result_equal)
#define BINARY_TREE_EMPTY_BRANCH ((struct binary_tree *)NULL)

#define binary_tree_init_root(root)             \
    do {                                        \
        (root)->parent = (root);                \
        (root)->left = (root);                  \
        (root)->right = (root);                 \
    } while (0)

#define binary_tree_init_node(node)                 \
    do {                                            \
        (node)->parent = BINARY_TREE_EMPTY_BRANCH;  \
        (node)->left = BINARY_TREE_EMPTY_BRANCH;    \
        (node)->right = BINARY_TREE_EMPTY_BRANCH;   \
        (node)->weight = 0;                         \
    } while (0)

#define binary_tree_root_node(node) ((node)->left == (node)->right && (node)->left != BINARY_TREE_EMPTY_BRANCH)
#define binary_tree_leaf_node(node) ((root)->left == BINARY_TREE_EMPTY_BRANCH && (root)->right == BINARY_TREE_EMPTY_BRANCH)
#define binary_tree_empty_root(root) ((root)->left == (root) && (root)->right == (root))
#define binary_tree_node(root) (binary_tree_root_node(root)?(root)->left:(root))

#define binary_tree_add(root, node, cmp)                            \
    do {                                                            \
        if (binary_tree_empty_root(root)) {                         \
            (root)->parent = (root)->left = (root)->right = (node); \
            (node)->parent = (root);                                \
        } else {                                                    \
            __binary_tree_add(root->left, node, cmp);               \
        }                                                           \
    } while (0)

#define binary_tree_detach(node)                                \
    do {                                                        \
        if (!(binary_tree_empty_root(node) ||                   \
              (node)->parent == BINARY_TREE_EMPTY_BRANCH)) {    \
            struct binary_tree *p = (node)->parent;             \
            if (binary_tree_root_node(p)) {                     \
                p->left = p->right = p;                         \
            } else {                                            \
                if (p->left == (node))                          \
                    p->left = BINARY_TREE_EMPTY_BRANCH;         \
                else                                            \
                    p->right = BINARY_TREE_EMPTY_BRANCH;        \
                while (!binary_tree_root_node(p)) {             \
                    p->weight -= (node)->weight + 1;            \
                    p = p->parent;                              \
                }                                               \
            }                                                   \
            (node)->parent = BINARY_TREE_EMPTY_BRANCH;          \
        }                                                       \
    } while (0)

/* TODO: binary_tree_remove */

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
            __binary_tree_search(root, key, match, results, limit); \
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
                    __binary_tree_traverse_infix(root->left, cbk, user_data); \
                    break;                                              \
                case binary_tree_traverse_type_prefix:                  \
                    __binary_tree_traverse_prefix(root->left, cbk, user_data); \
                    break;                                              \
                case binary_tree_traverse_type_postfix:                 \
                    __binary_tree_traverse_postfix(root->left, cbk, user_data); \
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

#endif
