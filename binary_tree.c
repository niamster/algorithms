#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "binary_tree.h"

#ifdef BINARY_TREE_AVL
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
#endif

void
__binary_tree_add(struct binary_tree *root,
                  struct binary_tree *node,
                  binary_tree_cmp_cbk_t cmp)
{
    cmp_result_t res;
    struct binary_tree **n, *r = root;
    int weight = node->weight + 1;

    do {
        res = cmp(r, node);
        r->weight += weight;

        n = BINARY_TREE_DIRECTION_LEFT(res)?&r->left:&r->right;

        if (*n == BINARY_TREE_EMPTY_BRANCH) {
            *n = node;
            (*n)->parent = r;

#ifdef BINARY_TREE_AVL
            binary_tree_avl_rebalance_on_insert(*n);
#endif

            break;
        }

        r = *n;
    } while (1);
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

#ifdef BINARY_TREE_AVL
        binary_tree_avl_rebalance_on_delete(node->parent);
#endif
    }

    node->parent = node;
}

void
__binary_tree_remove(struct binary_tree *node)
{
    struct binary_tree *p = node->parent;

    if (binary_tree_root_node(p)) {
        p->left = p->right = p;
    } else {
        struct binary_tree **pp = (p->left == node)?&p->left:&p->right;

        if (binary_tree_leaf_node(node)) {
            *pp = BINARY_TREE_EMPTY_BRANCH;
        } else if (node->left != BINARY_TREE_EMPTY_BRANCH &&
                   node->right == BINARY_TREE_EMPTY_BRANCH) {
            *pp = node->left;
        } else if (node->left == BINARY_TREE_EMPTY_BRANCH &&
                   node->right != BINARY_TREE_EMPTY_BRANCH) {
            *pp = node->right;
        } else {
            struct binary_tree **n = &node->right->left;

            while (*n != BINARY_TREE_EMPTY_BRANCH) {
                n = &(*n)->left;
            }
            *n = node->left;
            *pp = node->right;
        }
        while (!binary_tree_top(p)) {
            --p->weight;
            p = p->parent;
        }

#ifdef BINARY_TREE_AVL
        binary_tree_avl_rebalance_on_delete(node->parent);
#endif
    }

    node->parent = node;
    node->left = BINARY_TREE_EMPTY_BRANCH;
    node->right = BINARY_TREE_EMPTY_BRANCH;
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
    dot_dump_node(info->out, "binary_tree_node", info->id, name);
    if (root->left != BINARY_TREE_EMPTY_BRANCH) {
        dot_dump_link_node_to_node(info->out, "binary_tree_node", info->id, "binary_tree_node", info->id+1);
    }
    if (root->right != BINARY_TREE_EMPTY_BRANCH) {
        int weight = root->left!=BINARY_TREE_EMPTY_BRANCH?root->left->weight+1:0;
        dot_dump_link_node_to_node(info->out, "binary_tree_node", info->id, "binary_tree_node", info->id+weight+1);
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
contruct_binary_tree(int *array,
                     int count,
                     struct binary_tree *root)
{
    struct binary_tree_node *node;
    int i;

    binary_tree_init_root(root);

    for (i=0;i<count;++i) {
        if (!(node = malloc(sizeof(struct binary_tree_node)))) {
            fprintf(stderr, "Error error allocating %d bytes: %s", sizeof(struct binary_tree_node), strerror(errno));
            return -1;
        }

        binary_tree_init_node(&node->tree);

        node->num = array[i];
        binary_tree_add(root, &node->tree, binary_tree_integer_cmp);
    }
}

/* FIXME: use traverse? */
void
destroy_binary_tree(struct binary_tree *root)
{
    struct binary_tree *r, *left, *right;
    struct binary_tree_node *n;

    if (root == BINARY_TREE_EMPTY_BRANCH) /* sanity checks */
        return;

    if (binary_tree_empty_root(root))
        return;

    r = binary_tree_node(root);
    n = container_of(r, struct binary_tree_node, tree);

    left = r->left, right = r->right;

    binary_tree_remove(r);
    free(n);

    destroy_binary_tree(left);
    destroy_binary_tree(right);
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

    gettimeofday(&tb, NULL);
    contruct_binary_tree(array, count, &binary_tree_root);
    gettimeofday(&ta, NULL);
    usecs = ta.tv_sec*1000000 + ta.tv_usec - tb.tv_sec*1000000 - tb.tv_usec;
    secs = usecs/1000000;
    usecs %= 1000000;
    msecs = usecs/1000;
    usecs %= 1000;
    printf("Construct time: %lu seconds %lu msecs %lu usecs\n", secs, msecs, usecs);

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
    destroy_binary_tree(&binary_tree_root);
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
