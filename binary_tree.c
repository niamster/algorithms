#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "binary_tree.h"

#if defined(BINARY_TREE_AVL)
struct binary_tree *
binary_tree_avl_big_right_turn(struct binary_tree *node,
                               struct binary_tree *pivot,
                               struct binary_tree *bottom)
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

struct binary_tree *
binary_tree_avl_small_right_turn(struct binary_tree *node,
                                 struct binary_tree *pivot)
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

struct binary_tree *
binary_tree_avl_big_left_turn(struct binary_tree *node,
                              struct binary_tree *pivot,
                              struct binary_tree *bottom)
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

struct binary_tree *
binary_tree_avl_small_left_turn(struct binary_tree *node,
                                struct binary_tree *pivot)
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

void
binary_tree_avl_rebalance_on_insert(struct binary_tree *node)
{
    struct binary_tree *p = node->parent, *n = node;

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

void
binary_tree_avl_rebalance_on_delete(struct binary_tree *node)
{
    struct binary_tree *p = node->parent, *n = node;

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
void
binary_tree_rb_rotate_left(struct binary_tree *node)
{
    struct binary_tree *p = node->parent, **pp0, **pp1, *r = node->right;

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

void
binary_tree_rb_rotate_right(struct binary_tree *node)
{
    struct binary_tree *p = node->parent, **pp0, **pp1, *l = node->left;

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

void
binary_tree_rb_rebalance_on_insert(struct binary_tree *node)
{
    struct binary_tree *p = node->parent, *gp, *unkle;

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
                binary_tree_rb_rotate_left(p);
                node = p;
                p = node->parent;
            } else if (node == p->left && p == gp->right) {
                binary_tree_rb_rotate_right(p);
                node = p;
                p = node->parent;
            }

            p->color = BINARY_TREE_RB_BLACK;
            gp->color = BINARY_TREE_RB_RED;

            if (gp->left == p)
                binary_tree_rb_rotate_right(gp);
            else
                binary_tree_rb_rotate_left(gp);

            break;
        }
    }
}

void
binary_tree_rb_rebalance_on_delete(struct binary_tree *node)
{
    while (!binary_tree_top(node) && node->color == BINARY_TREE_RB_BLACK) {
        struct binary_tree *p = node->parent;
        struct binary_tree *brother = (p->left == node?p->right:p->left);

        if (brother == BINARY_TREE_EMPTY_BRANCH)
            break;

        if (brother->color == BINARY_TREE_RB_RED) {
            brother->color = BINARY_TREE_RB_BLACK;
            p->color = BINARY_TREE_RB_RED;

            if (p->left == node) {
                binary_tree_rb_rotate_left(p);
            } else {
                binary_tree_rb_rotate_right(p);
            }
        } else {
            if (BINARY_TREE_RB_NODE_COLOR(brother->left) == BINARY_TREE_RB_BLACK &&
                    BINARY_TREE_RB_NODE_COLOR(brother->right) == BINARY_TREE_RB_BLACK) {
                brother->color = BINARY_TREE_RB_BLACK;
                node = p;
            } else if (BINARY_TREE_RB_NODE_COLOR(p->left == node?brother->left:brother->right) == BINARY_TREE_RB_BLACK) {
                brother->color = BINARY_TREE_RB_RED;

                if (p->left == node) {
                    binary_tree_rb_rotate_left(brother);
                } else {
                    binary_tree_rb_rotate_right(brother);
                }
            } else {
                brother->color = p->color;
                p->color = BINARY_TREE_RB_BLACK;

                if (p->left == node) {
                    binary_tree_rb_rotate_right(brother);
                } else {
                    binary_tree_rb_rotate_left(brother);
                }

                break;
            }
        }
    }
}
#endif

void
__binary_tree_add(struct binary_tree *root,
                  struct binary_tree *node,
                  binary_tree_cmp_cbk_t cmp)
{
    cmp_result_t res;
    struct binary_tree **n, *r = root;
    int weight = node->weight + 1;

    for (;;) {
        res = cmp(r, node);
        r->weight += weight;

        n = BINARY_TREE_DIRECTION_LEFT(res)?&r->left:&r->right;

        if (*n == BINARY_TREE_EMPTY_BRANCH) {
            *n = node;
            node->parent = r;

#if defined(BINARY_TREE_AVL)
            binary_tree_avl_rebalance_on_insert(node);
#elif defined(BINARY_TREE_RB)
            binary_tree_rb_rebalance_on_insert(node);
#endif

            break;
        }

        r = *n;
    }
}

void
__binary_tree_search(struct binary_tree *root,
                     void *key,
                     binary_tree_key_match_cbk_t match,
                     struct sllist *results,
                     int limit)
{
    cmp_result_t res;
    struct binary_tree *n = root;
    struct binary_tree_search_result *result;
    int matched = 0;

    do {
        res = match(n, key);

        if (res == cmp_result_equal) {
            if (!(result = malloc(sizeof(struct binary_tree_search_result)))) {
                fprintf(stderr, "Error error allocating %d bytes: %s", sizeof(struct binary_tree_search_result), strerror(errno));
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
__binary_tree_traverse_prefix(struct binary_tree *root,
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
__binary_tree_traverse_infix(struct binary_tree *root,
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
__binary_tree_traverse_postfix(struct binary_tree *root,
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
        struct binary_tree_search_result *node = container_of(e, struct binary_tree_search_result, list);
        sllist_detach(e, p);
        free(node);
    }
}

void
__binary_tree_detach(struct binary_tree *node)
{
    struct binary_tree *p = node->parent;

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
__binary_tree_remove(struct binary_tree *node)
{
    struct binary_tree *p, **pp0, **pp1;

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
        struct binary_tree *n = node->right;
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
            struct binary_tree *pn = n->parent, **ppn;
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
        struct binary_tree *n = node->right;

        while (n->left != BINARY_TREE_EMPTY_BRANCH) {
            n = n->left;
        }

        n->left = node->left;
        *pp0 = *pp1 = n;
        n->parent = node->parent;

        n->weight += node->left->weight + 1;
#endif
    }

    while (!binary_tree_top(p)) {
        --p->weight;
        p = p->parent;
    }

#if defined(BINARY_TREE_AVL)
    binary_tree_avl_rebalance_on_delete(node->parent);
#elif defined(BINARY_TREE_RB)
    if (node->color == BINARY_TREE_RB_BLACK && *pp0 != BINARY_TREE_EMPTY_BRANCH)
        binary_tree_rb_rebalance_on_delete(*pp0);
#endif

    node->parent = node;
    node->left = BINARY_TREE_EMPTY_BRANCH;
    node->right = BINARY_TREE_EMPTY_BRANCH;
    node->weight = 0;
#if defined(BINARY_TREE_AVL)
    node->balance = 0;
#elif defined(BINARY_TREE_RB)
    node->color = BINARY_TREE_RB_RED;
#endif
}

#ifdef BINARY_TREE_MAIN

#include <sys/time.h>
#include <getopt.h>

#include "dot.h"
#include "sllist.h"

struct binary_tree_node {
    int num;
    struct binary_tree tree;
};

cmp_result_t
binary_tree_integer_cmp(struct binary_tree *one,
                        struct binary_tree *two)
{
    struct binary_tree_node *_one = container_of(one, struct binary_tree_node, tree);
    struct binary_tree_node *_two = container_of(two, struct binary_tree_node, tree);

    return (cmp_result_t)int_sign(_two->num - _one->num);
}

struct binary_tree_dot_info {
    FILE *out;
    int id;
};

void
__dump_binary_tree_graph(struct binary_tree *root,
                         struct binary_tree_dot_info *info)
{
    char name[100];
    struct binary_tree_node *node = container_of(root, struct binary_tree_node, tree);

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
                       struct binary_tree *root)
{
    struct binary_tree_dot_info info = {
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

int
construct_binary_tree(int *array,
                      int count,
                      struct binary_tree *root,
                      struct binary_tree_node **pool)
{
    struct binary_tree_node *nodes;
    int i;

    binary_tree_init_root(root);

    if (!(*pool = malloc(count*sizeof(struct binary_tree_node)))) {
        fprintf(stderr, "Error error allocating %d bytes: %s", sizeof(struct binary_tree_node), strerror(errno));
        return -1;
    }

    nodes = *pool;

    for (i=0;i<count;++i) {
        binary_tree_init_node(&nodes[i].tree);

        nodes[i].num = array[i];
        binary_tree_add(root, &nodes[i].tree, binary_tree_integer_cmp);
    }
}

void
destroy_binary_tree(struct binary_tree *root,
                    struct binary_tree_node *pool)
{
    struct binary_tree *r;
    /* struct binary_tree_node *n; */

    if (root == BINARY_TREE_EMPTY_BRANCH) /* sanity checks */
        return;

    while (!binary_tree_empty_root(root)) {
        /* n = container_of(r, struct binary_tree_node, tree); */
        r = binary_tree_node(root);
        binary_tree_remove(r);
    }

  out:
    free(pool);
}

cmp_result_t
binary_tree_integer_match(struct binary_tree *node,
                          void *key)
{
    int _key = *(int *)key;
    struct binary_tree_node *_node = container_of(node, struct binary_tree_node, tree);

    return (cmp_result_t)int_sign(_key - _node->num);
}

void
search_binary_tree(struct binary_tree *root,
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
    unsigned long secs, msecs, usecs;

    int dump = 0;
    int count = 0;
    const char *input_data = "/dev/random";
    const char *graph = NULL;
    struct sllist values;
    int limit = -1;
    int key = -1;

    unsigned int *array;
    struct binary_tree binary_tree_root;
    struct binary_tree_node *pool;

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

    sllist_init(&values);

    {
        unsigned int i;
        for (i=0;i<count;++i) {
            search_binary_tree(&binary_tree_root, array[i], &values, 1);
            if (sllist_empty(&values)) {
                fprintf(stderr, "Element %u(%u) was not found\n", i, array[i]);
                /* goto out; */
            }
            binary_tree_search_results_free(&values);
        }
        printf("All elements of array were successfully found\n");
    }

    sllist_init(&values);

    gettimeofday(&tb, NULL);
    search_binary_tree(&binary_tree_root, key, &values, limit);
    gettimeofday(&ta, NULL);

    if (!sllist_empty(&values)) {
        struct sllist *e;
        int count = 0;
        sllist_for_each(&values, e) {
            ++count;
        }
        printf("Entries %d for key '%d'\n", count, key);
        binary_tree_search_results_free(&values);
    } else {
        printf("Value was not found for key '%d'\n", key);
    }

    usecs = ta.tv_sec*1000000 + ta.tv_usec - tb.tv_sec*1000000 - tb.tv_usec;
    secs = usecs/1000000;
    usecs %= 1000000;
    msecs = usecs/1000;
    usecs %= 1000;
    printf("Search time: %lu seconds %lu msecs %lu usecs\n", secs, msecs, usecs);

    if (graph) {
        dump_binary_tree_graph(graph, &binary_tree_root);
    }

  out:
    free(array);

    gettimeofday(&tb, NULL);
    destroy_binary_tree(&binary_tree_root, pool);
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
