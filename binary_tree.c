#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "binary_tree.h"

static void
binary_tree_rotate_left(struct binary_tree_node *node)
{
    struct binary_tree_node *p = node->parent, **pp, *r = node->right;

    pp = (p->left == node)?&p->left:&p->right;

#if defined(BINARY_TREE_RANDOM)
    node->weight -= r->weight + 1;
    r->weight += node->weight + 1;
#endif

    *pp = r;
    node->right = r->left;
    r->left = node;

    if (node->right) {
        node->right->parent = node;
#if defined(BINARY_TREE_RANDOM)
        node->weight += node->right->weight + 1;
#endif
    }

    r->parent = p;
    node->parent = r;
}

static void
binary_tree_rotate_right(struct binary_tree_node *node)
{
    struct binary_tree_node *p = node->parent, **pp, *l = node->left;

    pp = (p->left == node)?&p->left:&p->right;

#if defined(BINARY_TREE_RANDOM)
    node->weight -= l->weight + 1;
    l->weight += node->weight + 1;
#endif

    *pp = l;
    node->left = l->right;
    l->right = node;

    if (node->left) {
        node->left->parent = node;
#if defined(BINARY_TREE_RANDOM)
        node->weight += node->left->weight + 1;
#endif
    }

    l->parent = p;
    node->parent = l;
}

#if defined(BINARY_TREE_AVL)
static struct binary_tree_node *
binary_tree_avl_big_right_turn(struct binary_tree_node *node,
                               struct binary_tree_node *pivot,
                               struct binary_tree_node *bottom)
{
    if (node->parent->left == node)
        node->parent->left = bottom;
    else
        node->parent->right = bottom;
    bottom->parent = node->parent;

    if (bottom->balance == 0) {
        node->balance = pivot->balance = 0;
    } else if (bottom->balance == 1) {
        node->balance = 0;
        pivot->balance = -1;
    } else {
        node->balance = 1;
        pivot->balance = 0;
    }

    bottom->balance = 0;

    node->right = bottom->left;
    if (node->right != BINARY_TREE_EMPTY_BRANCH)
        node->right->parent = node;
    pivot->left = bottom->right;
    if (pivot->left != BINARY_TREE_EMPTY_BRANCH)
        pivot->left->parent = pivot;
    bottom->left = node;
    bottom->right = pivot;
    node->parent = bottom;
    pivot->parent = bottom;

    return bottom;
}

static struct binary_tree_node *
binary_tree_avl_small_right_turn(struct binary_tree_node *node,
                                 struct binary_tree_node *pivot)
{
    if (node->parent->left == node)
        node->parent->left = pivot;
    else
        node->parent->right = pivot;
    pivot->parent = node->parent;

    if (pivot->balance == 0) {
        node->balance = -1;
        pivot->balance = 1;
    } else {
        node->balance = pivot->balance = 0;
    }

    node->right = pivot->left;
    if (node->right != BINARY_TREE_EMPTY_BRANCH)
        node->right->parent = node;
    pivot->left = node;
    node->parent = pivot;

    return pivot;
}

static struct binary_tree_node *
binary_tree_avl_big_left_turn(struct binary_tree_node *node,
                              struct binary_tree_node *pivot,
                              struct binary_tree_node *bottom)
{
    if (node->parent->left == node)
        node->parent->left = bottom;
    else
        node->parent->right = bottom;
    bottom->parent = node->parent;

    if (bottom->balance == 0) {
        node->balance = pivot->balance = 0;
    } else if (bottom->balance == 1) {
        node->balance = -1;
        pivot->balance = 0;
    } else {
        node->balance = 0;
        pivot->balance = 1;
    }

    bottom->balance = 0;

    node->left = bottom->right;
    if (node->left != BINARY_TREE_EMPTY_BRANCH)
        node->left->parent = node;
    pivot->right = bottom->left;
    if (pivot->right != BINARY_TREE_EMPTY_BRANCH)
        pivot->right->parent = pivot;
    bottom->left = pivot;
    bottom->right = node;
    node->parent = bottom;
    pivot->parent = bottom;

    return bottom;
}

static struct binary_tree_node *
binary_tree_avl_small_left_turn(struct binary_tree_node *node,
                                struct binary_tree_node *pivot)
{
    if (node->parent->left == node)
        node->parent->left = pivot;
    else
        node->parent->right = pivot;
    pivot->parent = node->parent;

    if (pivot->balance == 0) {
        node->balance = 1;
        pivot->balance = -1;
    } else {
        node->balance = pivot->balance = 0;
    }
    pivot->parent = node->parent;

    node->left = pivot->right;
    if (node->left != BINARY_TREE_EMPTY_BRANCH)
        node->left->parent = node;
    pivot->right = node;
    node->parent = pivot;

    return pivot;
}

static void
binary_tree_avl_rebalance_on_insert(struct binary_tree_node *node)
{
    struct binary_tree_node *p = node->parent, *n = node;

    while (!binary_tree_root_node(p)) {
        if (p->left == n)
            ++p->balance;
        else
            --p->balance;

        if (p->balance == 0) {
            break;
        } else if (p->balance == -2) {
            if (p->right->balance == 1) {
                p = binary_tree_avl_big_right_turn(p, p->right, p->right->left);
            } else {
                p = binary_tree_avl_small_right_turn(p, p->right);
            }
        } else if (p->balance == 2) {
            if (p->left->balance == -1) {
                p = binary_tree_avl_big_left_turn(p, p->left, p->left->right);
            } else {
                p = binary_tree_avl_small_left_turn(p, p->left);
            }
        }

        n = p;
        p = p->parent;
    }
}

static void
binary_tree_avl_rebalance_on_delete(struct binary_tree_node *node)
{
#warning FIXME: AVL is not rebalanced after node deletion
#if 0
    struct binary_tree_node *p, *n;

    if (binary_tree_root_node(node))
        return;

    for (p=node;;) {
        if (p->balance == -1 || p->balance == 1) {
            break;
        } else if (p->balance == -2) {
            if (p->right->balance == 1) {
                p = binary_tree_avl_big_right_turn(p, p->right, p->right->left);
            } else {
                p = binary_tree_avl_small_right_turn(p, p->right);
            }
        } else if (p->balance == 2) {
            if (p->left->balance == -1) {
                p = binary_tree_avl_big_left_turn(p, p->left, p->left->right);
            } else {
                p = binary_tree_avl_small_left_turn(p, p->left);
            }
        }

        if (binary_tree_root_node(node->parent))
            break; // really?

        n = p;
        p = p->parent;

        if (p->left == n)
            --p->balance;
        else
            ++p->balance;
    }
#endif
}
#elif defined(BINARY_TREE_RB)
static void
binary_tree_rb_rebalance_on_insert(struct binary_tree_node *node)
{
    struct binary_tree_node *p = node->parent, *gp, *unkle;

    while (p->color == BINARY_TREE_RB_RED) {
        gp = p->parent;
        if (gp->left == p)
            unkle = gp->right;
        else
            unkle = gp->left;

        if (BINARY_TREE_RB_NODE_COLOR(unkle) == BINARY_TREE_RB_RED) {
            unkle->color = p->color = BINARY_TREE_RB_BLACK;

            if (binary_tree_root_node(gp->parent))
                break;

            gp->color = BINARY_TREE_RB_RED;

            node = gp;
            p = node->parent;
        } else {
            if (node == p->right && p == gp->left) {
                binary_tree_rotate_left(p);
                node = p;
                p = node->parent;
            } else if (node == p->left && p == gp->right) {
                binary_tree_rotate_right(p);
                node = p;
                p = node->parent;
            }

            p->color = BINARY_TREE_RB_BLACK;
            gp->color = BINARY_TREE_RB_RED;

            if (gp->left == p)
                binary_tree_rotate_right(gp);
            else
                binary_tree_rotate_left(gp);

            break;
        }
    }
}

static void
binary_tree_rb_rebalance_on_delete(struct binary_tree_node *node)
{
    if (node == BINARY_TREE_EMPTY_BRANCH)
        return;

    if (node->color == BINARY_TREE_RB_RED) {
        node->color = BINARY_TREE_RB_BLACK;
        return;
    }

    while (!binary_tree_root_node(node) && node->color == BINARY_TREE_RB_DOUBLE_BLACK) {
        struct binary_tree_node *p = node->parent;
        struct binary_tree_node *sibling;

        if (binary_tree_root_node(p))
            break;

        sibling = (p->right == node?p->left:p->right);

        if (sibling == BINARY_TREE_EMPTY_BRANCH)
            break;

        if (sibling->color == BINARY_TREE_RB_RED) {
            sibling->color = BINARY_TREE_RB_BLACK;
            p->color = BINARY_TREE_RB_RED;

            if (p->left == node) {
                binary_tree_rotate_left(p);
            } else {
                binary_tree_rotate_right(p);
            }
        } else {
            if (BINARY_TREE_RB_NODE_COLOR(sibling->left) == BINARY_TREE_RB_BLACK &&
                    BINARY_TREE_RB_NODE_COLOR(sibling->right) == BINARY_TREE_RB_BLACK) {
                sibling->color = BINARY_TREE_RB_BLACK;
                node = p;
            } else if (BINARY_TREE_RB_NODE_COLOR(p->left == node?sibling->left:sibling->right) == BINARY_TREE_RB_BLACK) {
                sibling->color = BINARY_TREE_RB_RED;

                if (p->left == node) {
                    binary_tree_rotate_left(sibling);
                } else {
                    binary_tree_rotate_right(sibling);
                }
            } else {
                sibling->color = p->color;
                p->color = BINARY_TREE_RB_BLACK;

                if (p->left == node) {
                    binary_tree_rotate_right(sibling);
                } else {
                    binary_tree_rotate_left(sibling);
                }

                break;
            }
        }
    }
}
#elif defined(BINARY_TREE_TREAP)
static void
binary_tree_treap_rebalance(struct binary_tree_node *node)
{
    struct binary_tree_node *p = node->parent;

    while (!binary_tree_root_node(node) && node->prio > p->prio) {
        if (p->left == node)
            binary_tree_rotate_right(p);
        else
            binary_tree_rotate_left(p);

        p = node->parent;
    }
}
#endif

#if defined(BINARY_TREE_RANDOM)
inline void
____binary_tree_add(struct binary_tree_node *root,
        struct binary_tree_node *node,
        binary_tree_cmp_cbk_t cmp)
{
    cmp_result_t res;
    struct binary_tree_node **n, *r = root, *dst = NULL;
    int weight = node->weight + 1;

    for (;;) {
        if (dst == NULL
                && get_random()%(1+r->weight) == 0)
            dst = r;

        r->weight += weight;

        res = cmp(r, node);

        n = BINARY_TREE_DIRECTION_LEFT(res)?&r->left:&r->right;

        if (*n == BINARY_TREE_EMPTY_BRANCH) {
            *n = node;
            node->parent = r;

            break;
        }

        r = *n;
    }

    if (dst) {
        r = node->parent;

        while (r != dst) {
            if (r->left == node)
                binary_tree_rotate_right(r);
            else
                binary_tree_rotate_left(r);

            r = node->parent;
        }
    }
}
#else
inline void
____binary_tree_add(struct binary_tree_node *root,
        struct binary_tree_node *node,
        binary_tree_cmp_cbk_t cmp)
{
    cmp_result_t res;
    struct binary_tree_node **n, *r = root;

    for (;;) {
        res = cmp(r, node);
        n = BINARY_TREE_DIRECTION_LEFT(res)?&r->left:&r->right;
        if (*n == BINARY_TREE_EMPTY_BRANCH) {
            *n = node;
            node->parent = r;

            break;
        }

        r = *n;
    }
}
#endif

void
__binary_tree_add(struct binary_tree_node *root,
        struct binary_tree_node *node,
        binary_tree_cmp_cbk_t cmp)
{
    ____binary_tree_add(binary_tree_node(root), node, cmp);

#if defined(BINARY_TREE_AVL)
    binary_tree_avl_rebalance_on_insert(node);
#elif defined(BINARY_TREE_RB)
    binary_tree_rb_rebalance_on_insert(node);
#endif
}

void
__binary_tree_add2(struct binary_tree_root *root,
        struct binary_tree_node *node,
        binary_tree_cmp_cbk_t cmp)
{
    struct binary_tree_node *p;

    ____binary_tree_add(binary_tree_node(&root->root), node, cmp);

#if defined(BINARY_TREE_RANDOM)
    if (cmp(root->leftmost, node) == cmp_result_less)
        root->leftmost = node;
    else if (cmp(root->rightmost, node) == cmp_result_greater)
        root->rightmost = node;
#else
    p = node->parent;
    if (p->left == node) {
        if (p == root->leftmost)
            root->leftmost = node;
    } else {
        if (p == root->rightmost)
            root->rightmost = node;
    }
#endif

#if defined(BINARY_TREE_AVL)
    binary_tree_avl_rebalance_on_insert(node);
#elif defined(BINARY_TREE_RB)
    binary_tree_rb_rebalance_on_insert(node);
#elif defined(BINARY_TREE_TREAP)
    binary_tree_treap_rebalance(node);
#endif
}

void
__binary_tree_search(struct binary_tree_node *root,
                     void *key,
                     binary_tree_key_match_cbk_t match,
                     struct sllist *results,
                     int limit)
{
    cmp_result_t res;
    struct binary_tree_node *n = root;
    struct binary_tree_node_search_result *result;
    int matched = 0;

    do {
        res = match(n, key);

        if (res == cmp_result_equal) {
            if (!(result = malloc(sizeof(struct binary_tree_node_search_result)))) {
                fprintf(stderr, "Error error allocating %d bytes: %s", sizeof(struct binary_tree_node_search_result), strerror(errno));
                break;
            }

            result->node = n;

            sllist_add(results, &result->list);

            matched = 1;

            if (limit != -1) {
                --limit;
                if (limit == 0)
                    break;
            }
        } else if (matched) {
            break;
        }

        n = BINARY_TREE_DIRECTION_LEFT(res)?n->left:n->right;
    } while (n != BINARY_TREE_EMPTY_BRANCH);
}

void
__binary_tree_traverse_prefix(struct binary_tree_node *root,
                              binary_tree_traverse_cbk_t cbk,
                              void *user_data)
{
    if (root == BINARY_TREE_EMPTY_BRANCH)
        return;

    cbk(root, user_data);
    __binary_tree_traverse_prefix(root->left, cbk, user_data);
    __binary_tree_traverse_prefix(root->right, cbk, user_data);
}

void
__binary_tree_traverse_infix(struct binary_tree_node *root,
                             binary_tree_traverse_cbk_t cbk,
                             void *user_data)
{
    if (root == BINARY_TREE_EMPTY_BRANCH)
        return;

     __binary_tree_traverse_infix(root->left, cbk, user_data);
    cbk(root, user_data);
    __binary_tree_traverse_infix(root->right, cbk, user_data);
}

void
__binary_tree_traverse_postfix(struct binary_tree_node *root,
                               binary_tree_traverse_cbk_t cbk,
                               void *user_data)
{
    if (root == BINARY_TREE_EMPTY_BRANCH)
        return;

    __binary_tree_traverse_postfix(root->left, cbk, user_data);
    __binary_tree_traverse_postfix(root->right, cbk, user_data);
    cbk(root, user_data);
}

void
binary_tree_search_results_free(struct sllist *results)
{
    struct sllist *e, *p, *t;
    sllist_for_each_safe_prev(results, e, p, t) {
        struct binary_tree_node_search_result *node = container_of(e, struct binary_tree_node_search_result, list);
        sllist_detach(e, p);
        free(node);
    }
}

void
__binary_tree_detach(struct binary_tree_node *node)
{
    struct binary_tree_node *p = node->parent;

    if (p->left == node)
        p->left = BINARY_TREE_EMPTY_BRANCH;
    else
        p->right = BINARY_TREE_EMPTY_BRANCH;

#if defined(BINARY_TREE_AVL)
    binary_tree_avl_rebalance_on_delete(node->parent);
#elif defined(BINARY_TREE_RB)
    binary_tree_rb_rebalance_on_delete(node->parent);
#elif defined(BINARY_TREE_RANDOM)
    while (!binary_tree_root_node(p)) {
        p->weight -= node->weight + 1;
        p = p->parent;
    }
#endif

    node->parent = node;
}

#if defined(BINARY_TREE_RB)
void
__binary_tree_remove(struct binary_tree_node *node)
{
    struct binary_tree_node *p, **pp;

  delete_node:
    p = node->parent;

    pp = (p->left == node)?&p->left:&p->right;

    if (binary_tree_leaf_node(node)) {
        *pp = BINARY_TREE_EMPTY_BRANCH;
    } else if (node->left != BINARY_TREE_EMPTY_BRANCH &&
            node->right == BINARY_TREE_EMPTY_BRANCH) {
        struct binary_tree_node *n = node->left;

        while (n->right != BINARY_TREE_EMPTY_BRANCH)
            n = n->right;

        *pp = n;

        if (n == node->left) {
            n->parent = p;
        } else {
            n->parent->right = n->left;
            if (n->left != BINARY_TREE_EMPTY_BRANCH)
                n->left->parent = n->parent;

            n->parent = p;

            n->left = node->left;
            node->left->parent = n;
        }
    } else if (node->left == BINARY_TREE_EMPTY_BRANCH &&
            node->right != BINARY_TREE_EMPTY_BRANCH) {
        struct binary_tree_node *n = node->right;

        while (n->left != BINARY_TREE_EMPTY_BRANCH)
            n = n->left;

        *pp = n;

        if (n == node->right) {
            n->parent = p;
        } else {
            n->parent->left = n->right;
            if (n->right != BINARY_TREE_EMPTY_BRANCH)
                n->right->parent = n->parent;

            n->parent = p;

            n->right = node->right;
            node->right->parent = n;
        }
    } else {
        struct binary_tree_node *n = node->right;

        while (n->left != BINARY_TREE_EMPTY_BRANCH)
            n = n->left;

        *pp = n;

        if (n == node->right) {
            node->left->parent = n;
            n->parent = node->parent;
            node->parent = n;
        } else {
            struct binary_tree_node *p = n->parent, **pp;
            pp = (p->left == n)?&p->left:&p->right;

            *pp = node;

            node->left->parent = node->right->parent = n;

            pswap((void *)&n->parent, (void *)&node->parent);
        }

        pswap((void *)&n->right, (void *)&node->right);

        n->left = node->left;
        node->left = BINARY_TREE_EMPTY_BRANCH;

        swap(&n->color, &node->color);

        goto delete_node;
    }

    binary_tree_rb_rebalance_on_delete(*pp);

    binary_tree_init_node(node);
}
#else
void
__binary_tree_remove(struct binary_tree_node *node)
{
    struct binary_tree_node *p, **pp;

    p = node->parent;

#if defined(BINARY_TREE_AVL)
    if (binary_tree_root_node(p)) {
        pp = &p->left;
    } else {
        if (p->left == node) {
            pp = &p->left;
            --p->balance;
        } else {
            pp = &p->right;
            ++p->balance;
        }
    }
#else
    pp = (p->left == node)?&p->left:&p->right;
#endif

    if (binary_tree_leaf_node(node)) {
        *pp = BINARY_TREE_EMPTY_BRANCH;
    } else if (node->left != BINARY_TREE_EMPTY_BRANCH &&
            node->right == BINARY_TREE_EMPTY_BRANCH) {
        /* FIXME: merge code from RB (random: weight, treap: prio, avl: balance) */
        *pp = node->left;
        (*pp)->parent = p;
    } else if (node->left == BINARY_TREE_EMPTY_BRANCH &&
            node->right != BINARY_TREE_EMPTY_BRANCH) {
        /* FIXME: merge code from RB (random: weight, treap: prio, avl: balance) */
        *pp = node->right;
        (*pp)->parent = p;
    } else {
#if defined(BINARY_TREE_RANDOM)
        if (node->left->weight+node->left->weight == 0
                || get_random()%(node->left->weight+node->left->weight) < node->left->weight) {

            struct binary_tree_node *n = node->left;
            void *t;

            while (n->right != BINARY_TREE_EMPTY_BRANCH) {
                n->weight += node->right->weight + 1;
                n = n->right;
            }

            *pp = node->left;
            (*pp)->parent = p;

            node->right->parent = n;
            n->right = node->right;

            n->weight += node->right->weight + 1;
        } else
#endif
        {
            struct binary_tree_node *n = node->right;
            while (n->left != BINARY_TREE_EMPTY_BRANCH) {
#if defined(BINARY_TREE_RANDOM)
                n->weight += node->left->weight + 1;
#endif
                n = n->left;
            }

            *pp = node->right;
            (*pp)->parent = p;

            node->left->parent = n;
            n->left = node->left;

#if defined(BINARY_TREE_AVL)
            // FIXME
#elif defined(BINARY_TREE_RANDOM)
            n->weight += node->left->weight + 1;
#elif defined(BINARY_TREE_TREAP)
            binary_tree_treap_rebalance(n->left);
#endif
        }
    }

#if defined(BINARY_TREE_AVL)
    binary_tree_avl_rebalance_on_delete(p);
#elif defined(BINARY_TREE_RANDOM)
    while (!binary_tree_root_node(p)) {
        --p->weight;
        p = p->parent;
    }
#endif

    binary_tree_init_node(node);
}
#endif

#ifdef BINARY_TREE_MAIN

#include <sys/time.h>
#include <getopt.h>

#include "dot.h"
#include "sllist.h"

struct bt_node {
    int num;
    struct binary_tree_node tree;
};

cmp_result_t
binary_tree_integer_cmp(struct binary_tree_node *one,
                        struct binary_tree_node *two)
{
    struct bt_node *_one = container_of(one, struct bt_node, tree);
    struct bt_node *_two = container_of(two, struct bt_node, tree);

    return (cmp_result_t)int_sign(_two->num - _one->num);
}

struct binary_tree_node_dot_info {
    FILE *out;
    unsigned long id;
};

void
__dump_binary_tree_graph(struct binary_tree_node *root,
                         struct binary_tree_node_dot_info *info)
{
    char name[100];
    struct bt_node *node = container_of(root, struct bt_node, tree);

#if defined(BINARY_TREE_AVL)
    sprintf(name, "%u:%d", node->num, root->balance);
#elif defined(BINARY_TREE_RANDOM)
    sprintf(name, "%u:%u", node->num, root->weight);
#elif defined(BINARY_TREE_TREAP)
    sprintf(name, "%u:%u", node->num, root->prio);
#else
    sprintf(name, "%u", node->num);
#endif

#if defined(BINARY_TREE_RB)
    if (root->color == BINARY_TREE_RB_RED)
        dot_dump_shape_colored(info->out, "binary_tree_node", (unsigned long)root, name, "black", "red", "red", NULL);
    else
        dot_dump_shape_colored(info->out, "binary_tree_node", (unsigned long)root, name, "red", "black", "black", NULL);
#else
    dot_dump_node(info->out, "binary_tree_node", (unsigned long)root, name);
#endif

    if (root->left != BINARY_TREE_EMPTY_BRANCH) {
        dot_dump_link_node_to_node(info->out, "binary_tree_node", (unsigned long)root, "binary_tree_node", (unsigned long)root->left);
    } else {
#if defined(BINARY_TREE_RB)
        dot_dump_shape_colored(info->out, "NULL", info->id, "", "red", "black", "black", "box");
#else
        dot_dump_shape_colored(info->out, "NULL", info->id, "", "red", "black", "black", "circle");
#endif
        dot_dump_link_node_to_node(info->out, "binary_tree_node", (unsigned long)root, "NULL", (unsigned long)info->id);
        ++info->id;
    }
    if (root->right != BINARY_TREE_EMPTY_BRANCH) {
        dot_dump_link_node_to_node(info->out, "binary_tree_node", (unsigned long)root, "binary_tree_node", (unsigned long)root->right);
    } else {
#if defined(BINARY_TREE_RB)
        dot_dump_shape_colored(info->out, "NULL", info->id, "", "red", "black", "black", "box");
#else
        dot_dump_shape_colored(info->out, "NULL", info->id, "", "red", "black", "black", "circle");
#endif
        dot_dump_link_node_to_node(info->out, "binary_tree_node", (unsigned long)root, "NULL", (unsigned long)info->id);
        ++info->id;
    }
}

void
dump_binary_tree_graph(const char *graph,
                       struct binary_tree_node *root)
{
    struct binary_tree_node_dot_info info = {
        .out = fopen(graph, "w+"),
        .id = 0,
    };

    if (!info.out)
        return;

    dot_dump_begin(info.out, "binary_tree", dot_graph_direction_top_to_bottom);
    binary_tree_traverse(binary_tree_traverse_type_prefix,
                         root, (binary_tree_traverse_cbk_t)__dump_binary_tree_graph, (void *)&info);
    dot_dump_end(info.out);

    fclose(info.out);
}

void
dump_binary_tree_node(struct binary_tree_node *node, void *data)
{
    struct bt_node *n = container_of(node, struct bt_node, tree);
    printf("%d ", n->num);
}

int
construct_binary_tree(int *array,
                      int count,
                      struct binary_tree_root *root,
                      struct bt_node **pool)
{
    struct bt_node *nodes;
    int i;

    binary_tree_init_root(root);

    if (!(*pool = malloc(count*sizeof(struct bt_node)))) {
        fprintf(stderr, "Error error allocating %d bytes: %s", sizeof(struct bt_node), strerror(errno));
        return -1;
    }

    nodes = *pool;

    for (i=0;i<count;++i) {
        binary_tree_init_node(&nodes[i].tree);

        nodes[i].num = array[i];
        binary_tree_add2(root, &nodes[i].tree, binary_tree_integer_cmp);
    }
}

unsigned int
destroy_binary_tree(struct binary_tree_root *root,
                    struct bt_node *pool)
{
    struct binary_tree_node *r;
    unsigned int removed = 0;
    /* struct bt_node *n; */

    while (!binary_tree_empty_root(&root->root)) {
        r = binary_tree_node(&root->root);

        /* n = container_of(r, struct bt_node, tree); */
        /* printf("Removing: %d\n", n->num); */

        binary_tree_remove2(root, r);

        ++removed;

        /* { */
        /*     struct bt_node *lm = container_of(root->leftmost, struct bt_node, tree); */
        /*     struct bt_node *rm = container_of(root->rightmost, struct bt_node, tree); */

        /*     printf("Leftmost: %d, Rightmost: %d\n", */
        /*             (root->leftmost==BINARY_TREE_EMPTY_BRANCH)?-1:lm->num, */
        /*             (root->leftmost==BINARY_TREE_EMPTY_BRANCH)?-1:rm->num); */
        /* } */
    }

  out:
    free(pool);

    return removed;
}

cmp_result_t
binary_tree_integer_match(struct binary_tree_node *node,
                          void *key)
{
    int _key = *(int *)key;
    struct bt_node *_node = container_of(node, struct bt_node, tree);

    return (cmp_result_t)int_sign(_key - _node->num);
}

void
search_binary_tree(struct binary_tree_node *root,
                   int key,
                   struct sllist *result,
                   int limit)
{
    binary_tree_search(root, (void *)&key, binary_tree_integer_match, result, limit);
}

int
usage(const char *prog)
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "%s --key|-k key [--limit|-l limit] [--input-data|-i <path>] [--count|-c <num>] [--dump] [--graph|-g <path>]\n", prog);
    return 1;
}

int main(int argc, char **argv)
{
    struct timeval tb, ta;
    unsigned long long secs, msecs, usecs;

    int dump = 0;
    int count = 0;
    const char *input_data = "/dev/random";
    const char *graph = NULL;
    struct sllist values;
    int limit = -1;
    int key = -1;

    unsigned int removed;

    unsigned int *array;
    struct binary_tree_root binary_tree_root;
    struct bt_node *pool;

    int opt;
    const struct option options[] = {
        {"limit",         required_argument, NULL, 'l'},
        {"key",           required_argument, NULL, 'k'},
        {"input-data",    required_argument, NULL, 'i'},
        {"count",         required_argument, NULL, 'c'},
        {"dump",          no_argument,       &dump, 1},
        {"graph",         required_argument, NULL, 'g'},
        {0,               0,                 0,    0}
    };

    while ((opt = getopt_long(argc, argv, "i:c:g:l:k:", options, NULL)) != -1) {
        switch (opt) {
            case 'c':
                count = atoi(optarg);
                break;
            case 'i':
                input_data = optarg;
                break;
            case 'g':
                graph = optarg;
                break;
            case 'l':
                limit = atoi(optarg);
                break;
            case 'k':
                key = atoi(optarg);
                break;
            case 0:
                break;
            default:
                return usage(argv[0]);
        }
    }

    if (generate_array(&array, &count, input_data) == -1)
        return 1;
    printf("Elements: %d\n", count);
    if (dump)
        print_array(array, count);

    if (key == -1)
        key = array[count - 1];

    gettimeofday(&tb, NULL);
    construct_binary_tree(array, count, &binary_tree_root, &pool);
    gettimeofday(&ta, NULL);
    usecs = ta.tv_sec*1000000 + ta.tv_usec - tb.tv_sec*1000000 - tb.tv_usec;
    secs = usecs/1000000;
    usecs %= 1000000;
    msecs = usecs/1000;
    usecs %= 1000;
    printf("Construct time: %lu seconds %lu msecs %lu usecs\n", secs, msecs, usecs);

    {
        struct bt_node *lm = container_of(binary_tree_root.leftmost, struct bt_node, tree);
        struct bt_node *rm = container_of(binary_tree_root.rightmost, struct bt_node, tree);
        printf("Leftmost: %d, Rightmost: %d\n", lm->num, rm->num);
    }

    if (dump) {
        struct bt_node *n = container_of(binary_tree_node(&binary_tree_root.root),
                struct bt_node, tree);
        printf("Root: %d \n", n->num);
        printf("Infix walk:\n");
        binary_tree_traverse(binary_tree_traverse_type_infix,
                &binary_tree_root.root, dump_binary_tree_node, NULL);
        printf("\n");
    }

    {
        struct binary_tree_node_search_result *r;
        struct sllist *e;
        struct bt_node *n;
        unsigned int i;

        usecs = 0;
        for (i=0;i<count;++i) {
            sllist_init(&values);

            gettimeofday(&tb, NULL);
            search_binary_tree(&binary_tree_root.root, array[i], &values, 1);
            gettimeofday(&ta, NULL);

            usecs += ta.tv_sec*1000000 + ta.tv_usec - tb.tv_sec*1000000 - tb.tv_usec;

            if (sllist_empty(&values)) {
                fprintf(stderr, "Element %u(%u) was not found\n", i, array[i]);
                goto out;
            }

            e = sllist_first(&values);
            r = container_of(e, struct binary_tree_node_search_result, list);
            n = container_of(binary_tree_node(r->node), struct bt_node, tree);
            if (n->num != array[i]) {
                fprintf(stderr, "Found element %u does not match to the search criteria %u\n", n->num, array[i]);
                goto out;
            }

            binary_tree_search_results_free(&values);
        }

        secs = usecs/1000000;
        usecs %= 1000000;
        msecs = usecs/1000;
        usecs %= 1000;

        printf("All elements of array were successfully found, avg time: %lu seconds %lu msecs %lu usecs\n", secs, msecs, usecs);
    }

    {
        sllist_init(&values);

        gettimeofday(&tb, NULL);
        search_binary_tree(&binary_tree_root.root, key, &values, limit);
        gettimeofday(&ta, NULL);

        if (!sllist_empty(&values)) {
            struct binary_tree_node_search_result *r;
            struct sllist *e;
            struct bt_node *n;
            int count = 0;

            sllist_for_each(&values, e) {
                ++count;
                r = container_of(e, struct binary_tree_node_search_result, list);
                n = container_of(binary_tree_node(r->node), struct bt_node, tree);
                if (n->num != key) {
                    fprintf(stderr, "Found element %u does not match to the search criteria %u\n", n->num, key);
                    goto out;
                }
            }
            printf("%d node%s for key '%d'\n", count, count==1?"":"s", key);
            binary_tree_search_results_free(&values);
        } else {
            printf("Node was not found for key '%d'\n", key);
        }

        usecs = ta.tv_sec*1000000 + ta.tv_usec - tb.tv_sec*1000000 - tb.tv_usec;
        secs = usecs/1000000;
        usecs %= 1000000;
        msecs = usecs/1000;
        usecs %= 1000;
        printf("Search time: %lu seconds %lu msecs %lu usecs\n", secs, msecs, usecs);
    }

    {
        struct binary_tree_node_search_result *r;
        struct sllist *e;
        struct bt_node *n;
        unsigned int i;
        unsigned int pivot = count/2;

        for (i=0;i<pivot;++i) {
            sllist_init(&values);

            search_binary_tree(&binary_tree_root.root, array[i], &values, 1);

            if (sllist_empty(&values)) {
                fprintf(stderr, "Element %u(%u) was not found\n", i, array[i]);
                goto out;
            }

            e = sllist_first(&values);
            r = container_of(e, struct binary_tree_node_search_result, list);
            n = container_of(binary_tree_node(r->node), struct bt_node, tree);
            if (n->num != array[i]) {
                fprintf(stderr, "Found element %u does not match to the search criteria %u\n", n->num, array[i]);
                goto out;
            }

            // printf("Removing %u\n", n->num);
            binary_tree_remove2(&binary_tree_root, binary_tree_node(r->node));

            binary_tree_search_results_free(&values);
        }

        printf("Removed %d tree nodes\n", pivot);

        for (i=pivot;i<count;++i) {
            sllist_init(&values);

            search_binary_tree(&binary_tree_root.root, array[i], &values, 1);

            if (sllist_empty(&values)) {
                fprintf(stderr, "Element %u(%u) was not found\n", i, array[i]);
                goto out;
            }

            e = sllist_first(&values);
            r = container_of(e, struct binary_tree_node_search_result, list);
            n = container_of(binary_tree_node(r->node), struct bt_node, tree);
            if (n->num != array[i]) {
                fprintf(stderr, "Found element %u does not match to the search criteria %u\n", n->num, array[i]);
                goto out;
            }

            binary_tree_search_results_free(&values);
        }

        printf("Rest elements of array were successfully found\n");
    }

  out:
    if (graph) {
        dump_binary_tree_graph(graph, &binary_tree_root.root);
    }

    free(array);

    gettimeofday(&tb, NULL);
    removed = destroy_binary_tree(&binary_tree_root, pool);
    printf("Removed %u tree nodes\n", removed);
    gettimeofday(&ta, NULL);
    usecs = ta.tv_sec*1000000 + ta.tv_usec - tb.tv_sec*1000000 - tb.tv_usec;
    secs = usecs/1000000;
    usecs %= 1000000;
    msecs = usecs/1000;
    usecs %= 1000;
    printf("Destruction time: %lu seconds %lu msecs %lu usecs\n", secs, msecs, usecs);

    return 0;
}
#endif
