#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "binary_tree.h"

static void
binary_tree_rotate_left(struct binary_tree_node *node)
{
    struct binary_tree_node *p = node->parent, **pp0, **pp1, *r = node->right;

    pp0 = (p->left == node)?&p->left:&p->right;
    if (binary_tree_root_node(p))
        pp1 = &p->right;
    else
        pp1 = pp0;

    node->weight -= r->weight + 1;
    r->weight += node->weight + 1;

    *pp1 = *pp0 = r;
    node->right = r->left;
    r->left = node;

    if (node->right) {
        node->right->parent = node;
        node->weight += node->right->weight + 1;
    }

    r->parent = p;
    node->parent = r;
}

static void
binary_tree_rotate_right(struct binary_tree_node *node)
{
    struct binary_tree_node *p = node->parent, **pp0, **pp1, *l = node->left;

    pp0 = (p->left == node)?&p->left:&p->right;
    if (binary_tree_root_node(p))
        pp1 = &p->right;
    else
        pp1 = pp0;

    node->weight -= l->weight + 1;
    l->weight += node->weight + 1;

    *pp1 = *pp0 = l;
    node->left = l->right;
    l->right = node;

    if (node->left) {
        node->left->parent = node;
        node->weight += node->left->weight + 1;
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
    if (binary_tree_root_node(node->parent))
        node->parent->left = node->parent->right = bottom;
    else if (node->parent->left == node)
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

    node->weight -= pivot->weight + 1;
    if (bottom->left != BINARY_TREE_EMPTY_BRANCH)
        node->weight += bottom->left->weight + 1;
    pivot->weight -= bottom->weight + 1;
    if (bottom->right != BINARY_TREE_EMPTY_BRANCH)
        pivot->weight += bottom->right->weight + 1;
    bottom->weight = node->weight + pivot->weight + 2;

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
    if (binary_tree_root_node(node->parent))
        node->parent->left = node->parent->right = pivot;
    else if (node->parent->left == node)
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

    node->weight -= pivot->weight + 1;
    if (pivot->left!=BINARY_TREE_EMPTY_BRANCH) {
        node->weight += pivot->left->weight + 1;
        pivot->weight -= pivot->left->weight + 1;
    }
    pivot->weight += node->weight + 1;

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
    if (binary_tree_root_node(node->parent))
        node->parent->left = node->parent->right = bottom;
    else if (node->parent->left == node)
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

    node->weight -= pivot->weight + 1;
    if (bottom->right != BINARY_TREE_EMPTY_BRANCH)
        node->weight += bottom->right->weight + 1;
    pivot->weight -= bottom->weight + 1;
    if (bottom->left != BINARY_TREE_EMPTY_BRANCH)
        pivot->weight += bottom->left->weight + 1;
    bottom->weight = node->weight + pivot->weight + 2;

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
    if (binary_tree_root_node(node->parent))
        node->parent->left = node->parent->right = pivot;
    else if (node->parent->left == node)
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

    node->weight -= pivot->weight + 1;
    if (pivot->right != BINARY_TREE_EMPTY_BRANCH) {
        node->weight += pivot->right->weight + 1;
        pivot->weight -= pivot->right->weight + 1;
    }
    pivot->weight += node->weight + 1;

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

    while (!binary_tree_top(p)) {
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
    struct binary_tree_node *p = node->parent, *n = node;

    while (!binary_tree_top(p)) {
        if (p->left == n)
            --p->balance;
        else
            ++p->balance;

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

        n = p;
        p = p->parent;
    }
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

            if (binary_tree_top(gp->parent))
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
    while (!binary_tree_top(node) && node->color == BINARY_TREE_RB_BLACK) {
        struct binary_tree_node *p = node->parent;
        struct binary_tree_node *brother = (p->left == node?p->right:p->left);

        if (brother == BINARY_TREE_EMPTY_BRANCH)
            break;

        if (brother->color == BINARY_TREE_RB_RED) {
            brother->color = BINARY_TREE_RB_BLACK;
            p->color = BINARY_TREE_RB_RED;

            if (p->left == node) {
                binary_tree_rotate_left(p);
            } else {
                binary_tree_rotate_right(p);
            }
        } else {
            if (BINARY_TREE_RB_NODE_COLOR(brother->left) == BINARY_TREE_RB_BLACK &&
                    BINARY_TREE_RB_NODE_COLOR(brother->right) == BINARY_TREE_RB_BLACK) {
                brother->color = BINARY_TREE_RB_BLACK;
                node = p;
            } else if (BINARY_TREE_RB_NODE_COLOR(p->left == node?brother->left:brother->right) == BINARY_TREE_RB_BLACK) {
                brother->color = BINARY_TREE_RB_RED;

                if (p->left == node) {
                    binary_tree_rotate_left(brother);
                } else {
                    binary_tree_rotate_right(brother);
                }
            } else {
                brother->color = p->color;
                p->color = BINARY_TREE_RB_BLACK;

                if (p->left == node) {
                    binary_tree_rotate_right(brother);
                } else {
                    binary_tree_rotate_left(brother);
                }

                break;
            }
        }
    }
}
#elif defined(BINARY_TREE_RANDOM)
#if defined(BINARY_TREE_RANDOM_PREGEN) && BINARY_TREE_RANDOM_PREGEN > 0
static unsigned int *binary_tree_random = NULL;
static unsigned int binary_tree_idx = -1;

void binary_tree_generate_random(void) __attribute__((constructor));

void
binary_tree_generate_random(void)
{
    unsigned int count = BINARY_TREE_RANDOM_PREGEN;

    generate_array(&binary_tree_random, &count, "/dev/urandom");
}

static inline unsigned int
binary_tree_get_random(void)
{
    return binary_tree_random[++binary_tree_idx%BINARY_TREE_RANDOM_PREGEN];
}
#else
static inline unsigned int
binary_tree_get_random(void)
{
    return generate_random();
}
#endif
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
                && binary_tree_get_random()%(1+r->weight) == 0)
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
        n = &node->parent;

        while (*n != dst) {
            if ((*n)->left == node)
                binary_tree_rotate_right(*n);
            else
                binary_tree_rotate_left(*n);
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
    int weight = node->weight + 1;

    for (;;) {
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

    if (binary_tree_root_node(p)) {
        p->left = p->right = p;
    } else {
        if (p->left == node)
            p->left = BINARY_TREE_EMPTY_BRANCH;
        else
            p->right = BINARY_TREE_EMPTY_BRANCH;

        while (!binary_tree_top(p)) {
            p->weight -= node->weight + 1;
            p = p->parent;
        }

#if defined(BINARY_TREE_AVL)
        binary_tree_avl_rebalance_on_delete(node->parent);
#elif defined(BINARY_TREE_RB)
        binary_tree_rb_rebalance_on_delete(node->parent);
#endif
    }

    node->parent = node;
}

void
__binary_tree_remove(struct binary_tree_node *node)
{
    struct binary_tree_node *p, **pp0, **pp1;

#if defined(BINARY_TREE_RB)
  delete_node:
#endif
    p = node->parent;

    pp0 = (p->left == node)?&p->left:&p->right;
    if (binary_tree_root_node(p))
        pp1 = &p->right;
    else
        pp1 = pp0;

    if (binary_tree_leaf_node(node)) {
        *pp0 = *pp1 = (binary_tree_root_node(p)?p:BINARY_TREE_EMPTY_BRANCH);
    } else if (node->left != BINARY_TREE_EMPTY_BRANCH &&
            node->right == BINARY_TREE_EMPTY_BRANCH) {
        *pp0 = *pp1 = node->left;
        (*pp0)->parent = p;
    } else if (node->left == BINARY_TREE_EMPTY_BRANCH &&
            node->right != BINARY_TREE_EMPTY_BRANCH) {
        *pp0 = *pp1 = node->right;
        (*pp0)->parent = p;
    } else {
#if defined(BINARY_TREE_RB)
        struct binary_tree_node *n = node->right;
        void *t;

        while (n->left != BINARY_TREE_EMPTY_BRANCH) {
            n = n->left;
        }

        *pp0 = *pp1 = n;

        if (n == node->right) {
            node->left->parent = n;
            n->parent = node->parent;
            node->parent = n;
        } else {
            struct binary_tree_node *pn = n->parent, **ppn;
            ppn = (pn->left == n)?&pn->left:&pn->right;

            *ppn = node;

            node->left->parent = node->right->parent = n;

            /* pswap(&n->parent, &node->parent); */
            t = n->parent, n->parent = node->parent, node->parent = t;
        }

        /* pswap(&n->right, &node->right); */
        t = n->right, n->right = node->right, node->right = t;

        n->left = node->left;
        node->left = BINARY_TREE_EMPTY_BRANCH;

        swap(&n->color, &node->color);
        swap(&n->weight, &node->weight);

        goto delete_node;
#else
#if defined(BINARY_TREE_RANDOM)
        if (node->left->weight+node->left->weight == 0
                || binary_tree_get_random()%(node->left->weight+node->left->weight) < node->left->weight) {

            struct binary_tree_node *n = node->left;
            void *t;

            while (n->right != BINARY_TREE_EMPTY_BRANCH) {
                n = n->right;
            }

            *pp0 = *pp1 = node->left;
            (*pp0)->parent = p;

            node->right->parent = n;
            n->parent = p;
            n->right = node->right;

            n->weight += node->right->weight + 1;
        } else
#endif
        {
            struct binary_tree_node *n = node->right;

            while (n->left != BINARY_TREE_EMPTY_BRANCH) {
                n = n->left;
            }

            *pp0 = *pp1 = node->right;
            (*pp0)->parent = p;

            node->left->parent = n;
            n->parent = p;
            n->left = node->left;

            n->weight += node->left->weight + 1;
        }
#endif
    }

    while (!binary_tree_top(p)) {
        --p->weight;
        p = p->parent;
    }

#if defined(BINARY_TREE_AVL)
    binary_tree_avl_rebalance_on_delete(p);
#elif defined(BINARY_TREE_RB)
    if (node->color == BINARY_TREE_RB_BLACK && *pp0 != BINARY_TREE_EMPTY_BRANCH)
        binary_tree_rb_rebalance_on_delete(*pp0);
#endif

    binary_tree_init_node(node);
}

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
    int id;
};

void
__dump_binary_tree_graph(struct binary_tree_node *root,
                         struct binary_tree_node_dot_info *info)
{
    char name[100];
    struct bt_node *node = container_of(root, struct bt_node, tree);

    ++info->id;

    sprintf(name, "%u", node->num);
#if defined(BINARY_TREE_RB)
    if (root->color == BINARY_TREE_RB_RED)
        dot_dump_shape_colored(info->out, "binary_tree_node", info->id, name, "black", "red", "red", NULL);
    else
        dot_dump_shape_colored(info->out, "binary_tree_node", info->id, name, "red", "black", "black", NULL);
#else
    dot_dump_node(info->out, "binary_tree_node", info->id, name);
#endif

#if defined(BINARY_TREE_RB)
    sprintf(name, "binary_tree_empty_node_%u", node->num);
#endif

    if (root->left != BINARY_TREE_EMPTY_BRANCH) {
        dot_dump_link_node_to_node(info->out, "binary_tree_node", info->id, "binary_tree_node", info->id+1);
    } else {
#if defined(BINARY_TREE_RB)
        dot_dump_shape_colored(info->out, name, info->id+1, "", "red", "black", "black", "box");
        dot_dump_link_node_to_node(info->out, "binary_tree_node", info->id, name, info->id+1);
#endif
    }
    if (root->right != BINARY_TREE_EMPTY_BRANCH) {
        int weight = root->left!=BINARY_TREE_EMPTY_BRANCH?root->left->weight+1:0;
        dot_dump_link_node_to_node(info->out, "binary_tree_node", info->id, "binary_tree_node", info->id+weight+1);
    } else {
#if defined(BINARY_TREE_RB)
        dot_dump_shape_colored(info->out, name, info->id+2, "", "red", "black", "black", "box");
        dot_dump_link_node_to_node(info->out, "binary_tree_node", info->id, name, info->id+2);
#endif
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

        for (i=0;i<count/2;++i) {
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

            binary_tree_remove2(&binary_tree_root, binary_tree_node(r->node));

            binary_tree_search_results_free(&values);
        }

        printf("Removed %d tree nodes\n");

        for (i=count/2;i<count;++i) {
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
    printf("Removed: %u\n", removed);
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
