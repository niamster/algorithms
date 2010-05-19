#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "binary_tree.h"

void
__binary_tree_add(struct binary_tree *root,
                  struct binary_tree *node,
                  binary_tree_cmp_cbk_t cmp)
{
    cmp_result_t res;
    struct binary_tree **n, *r = root;

    do {
        res = cmp(r, node);
        ++r->weight;

        n = BINARY_TREE_DIRECTION_LEFT(res)?&r->left:&r->right;

        if (*n == BINARY_TREE_EMPTY_BRANCH) {
            *n = node;
            (*n)->parent = r;

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

void
destroy_binary_tree(struct binary_tree *root)
{
    struct binary_tree *r;
    struct binary_tree_node *n;

    if (root == BINARY_TREE_EMPTY_BRANCH) /* sanity checks */
        return;

    if (binary_tree_empty_root(root))
        return;

    r = binary_tree_node(root);
    n = container_of(r, struct binary_tree_node, tree);

    if (r->left != BINARY_TREE_EMPTY_BRANCH)
        destroy_binary_tree(r->left);
    if (r->right != BINARY_TREE_EMPTY_BRANCH)
        destroy_binary_tree(r->right);

    binary_tree_detach(r);

    free(n);
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

/*
  dd if=/dev/urandom of=/tmp/rnd count=1000000 bs=4
  ./binary-tree --input-data /tmp/rnd
*/

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
        printf("Entries for key '%d'\n", key);
        sllist_for_each(&values, e) {
            struct binary_tree_search_result *node = container_of(e, struct binary_tree_search_result, list);
            struct binary_tree_node *_node = container_of(node->node, struct binary_tree_node, tree);
            printf("    %d\n", _node->num);
        }
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
    destroy_binary_tree(&binary_tree_root);

    return 0;
}
#endif
