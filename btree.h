#ifndef _BTREE_H_
#define _BTREE_H_

#include <stdbool.h>

#include "helpers.h"
#include "sllist.h"

#if !defined(BTREE_ORDER)
#define BTREE_ORDER 5
#elif BTREE_ORDER < 3
#error btree order is less than 3
#endif

#define BTREE_ASSERT(x, d) ALGO_ASSERT("btree", x, d)

struct btree_root;

typedef cmp_result_t (*btree_cmp_cbk_t)(void *, void *);
typedef void (*btree_assign_cbk_t)(void *, void *);
typedef struct btree_node * (*btree_zalloc_cbk_t)(struct btree_root *root);
typedef void (*btree_free_cbk_t)(struct btree_node *);

typedef void (*btree_traverse_cbk_t)(struct btree_node *, void *);

struct btree_node {
    struct btree_node *parent;
    struct btree_node *child[BTREE_ORDER];
    unsigned long valid;
    L1_ALIGNED;
    char data[0];
};

struct btree_root {
    struct btree_node *root;
    unsigned long size;
    unsigned long total;
    btree_cmp_cbk_t cmp;
    btree_assign_cbk_t assign;
    btree_zalloc_cbk_t zalloc;
    btree_free_cbk_t free;
};

#define BTREE_EMPTY_BRANCH  ((struct btree_node *)NULL)

#define btree_empty_root(r) ((r)->root == BTREE_EMPTY_BRANCH)

static inline void btree_init_root(struct btree_root *root,
        unsigned long size,
        btree_cmp_cbk_t cmp,
        btree_assign_cbk_t assign,
        btree_zalloc_cbk_t zalloc,
        btree_free_cbk_t free)
{
    unsigned int i;

    BTREE_ASSERT(BTREE_ORDER/2 != 0, "only odd btree order is supported");

    root->root = BTREE_EMPTY_BRANCH;
    root->size = size;
    root->total = size*(BTREE_ORDER - 1);
    root->cmp = cmp;
    root->assign = assign;
    root->zalloc = zalloc;
    root->free = free;
}

typedef enum {
    BTREE_ADD_SUCCESS,
    BTREE_ADD_DUPLICATE,
    BTREE_ADD_ALLOC,
} btree_add_result_t;

btree_add_result_t btree_add(struct btree_root *root, void *data);

bool btree_remove(struct btree_root *root, void *data);

bool btree_search(struct btree_root *root, void *key, void **data);

typedef enum {
    btree_traverse_type_prefix,
    btree_traverse_type_postfix
} btree_traverse_type_t;

void __btree_traverse_prefix(struct btree_node *nodep,
        btree_traverse_cbk_t cbk,
        void *data);

void __btree_traverse_postfix(struct btree_node *node,
        btree_traverse_cbk_t cbk,
        void *data);

static inline void btree_traverse(btree_traverse_type_t type,
        struct btree_root *root,
        btree_traverse_cbk_t cbk,
        void *data)
{
    if (BTREE_EMPTY_BRANCH != root->root) {
        switch (type) {
            case btree_traverse_type_prefix:
                __btree_traverse_prefix(root->root, cbk, data);
                break;
            case btree_traverse_type_postfix:
                __btree_traverse_postfix(root->root, cbk, data);
                break;
        }
    }
}

#endif
