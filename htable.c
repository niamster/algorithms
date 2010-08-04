#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <errno.h>
#include <getopt.h>

#include "helpers.h"
#include "dot.h"
#include "sllist.h"
#if defined(HTABLE_LIST)
/* nothing to be included */
#elif defined(HTABLE_TREE)
#include "binary_tree.h"
#else
#error Unknown storage type for handling hash table data
#endif

typedef unsigned int (*hash_function_t)(const char *, unsigned int len, unsigned int prime);

struct hash_node {
    struct key_value *data;
#if defined(HTABLE_LIST)
    struct sllist list;
#elif defined(HTABLE_TREE)
    struct binary_tree tree;
#endif
};

#if defined(HTABLE_TREE)
cmp_result_t
binary_tree_str_key_cmp(struct binary_tree *one,
                        struct binary_tree *two)
{
    struct hash_node *_one = container_of(one, struct hash_node, tree);
    struct hash_node *_two = container_of(two, struct hash_node, tree);

    return (cmp_result_t)int_sign(strcmp(_two->data->key, _one->data->key));
}
#endif

#if defined(HTABLE_LIST)
void
print_htable_entries(struct sllist *head)
{
    struct sllist *e;

    sllist_for_each(head, e) {
        struct hash_node *node = container_of(e, struct hash_node, list);
        printf("    '%s' => '%s'\n", node->data->key, node->data->value);
    }
}
#elif defined(HTABLE_TREE)
void
__print_htable_binary_tree_entries(struct binary_tree *root,
                                   void *__unused__)
{
    struct hash_node *node = container_of(root, struct hash_node, tree);
    printf("    '%s' => '%s'\n", node->data->key, node->data->value);
}

void
print_htable_entries(struct binary_tree *root)
{
    binary_tree_traverse(binary_tree_traverse_type_infix,
                         root, __print_htable_binary_tree_entries, NULL);
}

void
print_htable_binary_tree_search_result(struct sllist *head)
{
    struct sllist *e;

    sllist_for_each(head, e) {
        struct binary_tree_search_result *sr = container_of(e, struct binary_tree_search_result, list);
        struct hash_node *node = container_of(sr->node, struct hash_node, tree);
        printf("    '%s' => '%s'\n", node->data->key, node->data->value);
    }
}
#endif

void
print_htable(
#if defined(HTABLE_LIST)
               struct sllist *hash_table,
#elif defined(HTABLE_TREE)
               struct binary_tree *hash_table,
#endif
             int hash_size)
{
    printf("htable of %d entries\n", hash_size);

    int i;
    for (i=0;i<hash_size;++i) {
        printf("Level %d\n", i);

        print_htable_entries(&hash_table[i]);
    }
}

#ifdef HTABLE_TREE
struct binary_tree_dot_info {
    FILE *out;
    int id;
};

void
__dump_htable_binary_tree_graph(struct binary_tree *root,
                                struct binary_tree_dot_info *info)
{
    struct hash_node *node = container_of(root, struct hash_node, tree);

    ++info->id;

    dot_dump_node(info->out, "binary_tree_node", info->id, node->data->key);
    if (root->left != BINARY_TREE_EMPTY_BRANCH) {
        dot_dump_link_node_to_node(info->out, "binary_tree_node", info->id, "binary_tree_node", info->id+1);
    }
    if (root->right != BINARY_TREE_EMPTY_BRANCH) {
        int weight = root->left!=BINARY_TREE_EMPTY_BRANCH?root->left->weight+1:0;
        dot_dump_link_node_to_node(info->out, "binary_tree_node", info->id, "binary_tree_node", info->id+weight+1);
    }
}
#endif

void
dump_htable_graph(const char *graph,
#if defined(HTABLE_LIST)
                  struct sllist *hash_table,
#elif defined(HTABLE_TREE)
                  struct binary_tree *hash_table,
#endif
                  int hash_size)
{
    int i;
    FILE *out = fopen(graph, "w+");
#if defined(HTABLE_TREE)
    int shift = 0;
#endif

    if (!out)
        return;

    dot_dump_begin(out, "htable", dot_graph_direction_left_to_right);
    dot_dump_table(out, "htable", hash_size);
    for (i=0;i<hash_size;++i) {
#if defined(HTABLE_LIST)
        if (!sllist_empty(&hash_table[i])) {
            dot_dump_sllist(out, "list", i, &hash_table[i], struct hash_node, list, data->key);
            dot_dump_link_table_to_sllist_head(out, "htable", i, "list", i);
        }
#elif defined(HTABLE_TREE)
        struct binary_tree_dot_info info = {
            .out = out,
            .id = shift
        };
        binary_tree_traverse(binary_tree_traverse_type_prefix,
                             &hash_table[i], (binary_tree_traverse_cbk_t)__dump_htable_binary_tree_graph, (void *)&info);
        dot_dump_link_table_to_node(out, "htable", i, "binary_tree_node", shift + 1);
        shift += binary_tree_node(&hash_table[i])->weight + 1;
#endif
    }
    dot_dump_end(out);

    fclose(out);
}

void
print_htable_summary(
#if defined(HTABLE_LIST)
                     struct sllist *hash_table,
#elif defined(HTABLE_TREE)
                     struct binary_tree *hash_table,
#endif
                     int hash_size,
                     int count)
{
    printf("htable of %d entries\n", hash_size);

    int i;
    for (i=0;i<hash_size;++i) {
        int level_count = 0;

#if defined(HTABLE_LIST)
        struct sllist *e;
        sllist_for_each(&hash_table[i], e) {
            ++level_count;
        }
#elif defined(HTABLE_TREE)
        level_count = binary_tree_node(&hash_table[i])->weight + 1;
#endif

        printf("Level %5d: %5d entries(%.2f%%)\n", i, level_count, ((float)level_count/(float)count)*100);
    }
}

int
construct_htable(struct key_value *data,
                 int count,
#if defined(HTABLE_LIST)
                 struct sllist **hash_table,
#elif defined(HTABLE_TREE)
                 struct binary_tree **hash_table,
#endif
                 int hash_size,
                 hash_function_t hash_function,
                 struct hash_node **pool)
{
    int i;
    struct hash_node *nodes;

#if defined(HTABLE_LIST)
    if (!(*hash_table = malloc(hash_size*sizeof(struct sllist)))) {
        fprintf(stderr, "Error error allocating %d bytes: %s", hash_size*sizeof(struct sllist), strerror(errno));
        return -1;
    }
#elif defined(HTABLE_TREE)
    if (!(*hash_table = malloc(hash_size*sizeof(struct binary_tree)))) {
        fprintf(stderr, "Error error allocating %d bytes: %s", hash_size*sizeof(struct binary_tree), strerror(errno));
        return -1;
    }
#endif

    for (i=0;i<hash_size;++i) {
#if defined(HTABLE_LIST)
        sllist_init(&(*hash_table)[i]);
#elif defined(HTABLE_TREE)
        binary_tree_init_root(&(*hash_table)[i]);
#endif
    }

    if (!(*pool = malloc(count*sizeof(struct hash_node)))) {
        fprintf(stderr, "Error error allocating %d bytes: %s", count*sizeof(struct hash_node), strerror(errno));
        return -1;
    }

    nodes = *pool;

    for (i=0;i<count;++i) {
        int hash = hash_function(data[i].key, data[i].key?strlen(data[i].key):0, hash_size);
        if (hash >= hash_size) { /* sanity checks */
            fprintf(stderr, "Invalid hash: %d for %s, skipping element\n", hash, data[i].key);
            continue;
        }

        nodes[i].data = &data[i];
#if defined(HTABLE_LIST)
        sllist_add(&(*hash_table)[hash], &nodes[i].list);
#elif defined(HTABLE_TREE)
        binary_tree_init_node(&nodes[i].tree);
        binary_tree_add(&(*hash_table)[hash], &nodes[i].tree, binary_tree_str_key_cmp);
#endif
    }
}

#if defined(HTABLE_LIST)
void
destroy_htable_entries(struct sllist *head)
{
    struct sllist *e, *p, *t;
    sllist_for_each_safe_prev(head, e, p, t) {
        struct hash_node *node = container_of(e, struct hash_node, list);
        sllist_detach(e, p);
    }
}
#elif defined(HTABLE_TREE)
/* FIXME: use traverse? */
void
destroy_htable_entries(struct binary_tree *root)
{
    struct binary_tree *r, *left, *right;
    struct hash_node *n;

    if (root == BINARY_TREE_EMPTY_BRANCH) /* sanity checks */
        return;

    if (binary_tree_empty_root(root))
        return;

    r = binary_tree_node(root);
    n = container_of(r, struct hash_node, tree);

    left = r->left, right = r->right;

    binary_tree_remove(r);

    destroy_htable_entries(left);
    destroy_htable_entries(right);
}
#endif

void
destroy_htable(
#if defined(HTABLE_LIST)
               struct sllist *hash_table,
#elif defined(HTABLE_TREE)
               struct binary_tree *hash_table,
#endif
               int hash_size,
               struct hash_node *pool)
{
    int i;
    for (i=0;i<hash_size;++i)
        destroy_htable_entries(&hash_table[i]);

    free(pool);
    free(hash_table);
}

#if defined(HTABLE_TREE)
cmp_result_t
binary_tree_str_key_match(struct binary_tree *node,
                          void *key)
{
    struct hash_node *_node = container_of(node, struct hash_node, tree);

    if (!key)
        return _node->data->key?cmp_result_greater:cmp_result_equal;

    return (cmp_result_t)int_sign(strcmp(key, _node->data->key));
}
#endif

void
search_htable(
#if defined(HTABLE_LIST)
               struct sllist *hash_table,
#elif defined(HTABLE_TREE)
               struct binary_tree *hash_table,
#endif
              int hash_size,
              hash_function_t hash_function,
              const unsigned char *key,
              struct sllist *result,
              int limit)
{
#if defined(HTABLE_LIST)
    struct sllist *e;
#endif

    int hash = hash_function(key, key?strlen(key):0, hash_size);
    if (hash >= hash_size) { /* sanity checks */
        fprintf(stderr, "Invalid hash: %d for %s\n", hash, key);
        return;
    }

#if defined(HTABLE_LIST)
    sllist_for_each (&hash_table[hash], e) {
        struct hash_node *needle;
        struct hash_node *node = container_of(e, struct hash_node, list);

        if (!key) {
            if (node->data->key)
                continue;
        } else if (strcmp(key, node->data->key)) {
            continue;
        }

        if (!(needle = malloc(sizeof(struct hash_node)))) {
            fprintf(stderr, "Error error allocating %d bytes: %s", sizeof(struct hash_node), strerror(errno));
            break;
        }
        needle->data = node->data;
        sllist_add(result, &needle->list);

        if (limit != -1) {
            --limit;
            if (limit == 0)
                break;
        }
    }
#elif defined(HTABLE_TREE)
    binary_tree_search(&hash_table[hash], (void *)key, binary_tree_str_key_match, result, limit);
#endif
}

unsigned int
additive_hash(const char *key,
              unsigned int len,
              unsigned int prime)
{
    unsigned int hash = len;
	int c;

    if (!key)
        return 0;

	while (c = *key++)
	    hash += c;

	return hash % prime;
}

unsigned int
rotating_hash(const char *key,
              unsigned int len,
              unsigned int prime)
{
  unsigned int hash = len;
  int c;

  if (!key)
      return 0;

  while (c = *key++)
    hash = (hash<<4)^(hash>>28)^c;

  return hash % prime;
}


int
usage(const char *prog)
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "%s --hash-function|-f simple --hash-size|-s size --key|-k key [--limit|-l limit] [--input-data|-i <path>] [--count|-c <num>] [--dump-data] [--dump-htable] [--dump-htable-summary] [--graph|-g <path>]\n", prog);
    return 1;
}

int main(int argc, char **argv)
{
    struct timeval tb, ta;
    unsigned long secs, msecs, usecs;

    int dump_data = 0, dump_htable = 0, dump_htable_summary = 0;
    hash_function_t hash_function = NULL;
    int hash_size = 0;
#if defined(HTABLE_LIST)
    struct sllist *hash_table = NULL;
#elif defined(HTABLE_TREE)
    struct binary_tree *hash_table = NULL;
#endif
    struct hash_node *pool;
    int count = 0;

    const char *input_data = "/dev/random";
    struct key_value *data;
    const unsigned char *key = NULL;
    struct sllist values;
    int limit = -1;
    const char *graph = NULL;

    int opt;
    const struct option options[] = {
        {"hash-function", required_argument, NULL, 'f'},
        {"hash-size",     required_argument, NULL, 's'},
        {"key",           required_argument, NULL, 'k'},
        {"limit",         required_argument, NULL, 'l'},
        {"input-data",    required_argument, NULL, 'i'},
        {"count",         required_argument, NULL, 'c'},
        {"dump-htable",    no_argument,       &dump_htable, 1},
        {"dump-htable-summary",  no_argument, &dump_htable_summary, 1},
        {"dump-data",     no_argument,       &dump_data, 1},
        {"graph",         required_argument, NULL, 'g'},
        {0,               0,                 0,    0}
    };

    while ((opt = getopt_long(argc, argv, "f:s:k:l:i:c:g:", options, NULL)) != -1) {
        switch (opt) {
            case 'f':
                if (!strncmp(optarg, "additive", 8))
                    hash_function = additive_hash;
                else if (!strncmp(optarg, "rotating", 8))
                    hash_function = rotating_hash;
                else
                    return usage(argv[0]);
                break;
            case 'k':
                key = optarg;
                break;
            case 'l':
                limit = atoi(optarg);
                break;
            case 'c':
                count = atoi(optarg);
                break;
            case 's':
                hash_size = atoi(optarg);
                break;
            case 'i':
                input_data = optarg;
                break;
            case 'g':
                graph = optarg;
                break;
            case 0:
                break;
            default:
                return usage(argv[0]);
        }
    }
    if (hash_function == NULL || hash_size == 0)
        return usage(argv[0]);

    if (generate_key_value(&data, &count, input_data) == -1)
        return 1;
    printf("Elements: %d\n", count);
    if (dump_data)
        print_key_value(data, count);

    gettimeofday(&tb, NULL);
    construct_htable(data, count, &hash_table, hash_size, hash_function, &pool);
    gettimeofday(&ta, NULL);
    usecs = ta.tv_sec*1000000 + ta.tv_usec - tb.tv_sec*1000000 - tb.tv_usec;
    secs = usecs/1000000;
    usecs %= 1000000;
    msecs = usecs/1000;
    usecs %= 1000;
    printf("Construct time: %lu seconds %lu msecs %lu usecs\n", secs, msecs, usecs);

    if (dump_htable)
        print_htable(hash_table, hash_size);
    if (dump_htable_summary)
        print_htable_summary(hash_table, hash_size, count);

    sllist_init(&values);

    gettimeofday(&tb, NULL);
    search_htable(hash_table, hash_size, hash_function, key, &values, limit);
    gettimeofday(&ta, NULL);

    if (!sllist_empty(&values)) {
        printf("Entries for key '%s'\n", key);
#if defined(HTABLE_LIST)
        print_htable_entries(&values);
        destroy_htable_entries(&values);
#elif defined(HTABLE_TREE)
        print_htable_binary_tree_search_result(&values);
        binary_tree_search_results_free(&values);
#endif
    } else {
        printf("Value was not found for key '%s'\n", key);
    }

    usecs = ta.tv_sec*1000000 + ta.tv_usec - tb.tv_sec*1000000 - tb.tv_usec;
    secs = usecs/1000000;
    usecs %= 1000000;
    msecs = usecs/1000;
    usecs %= 1000;
    printf("Search time: %lu seconds %lu msecs %lu usecs\n", secs, msecs, usecs);

    if (graph) {
        dump_htable_graph(graph, hash_table, hash_size);
    }

  out:
    free(data);

    gettimeofday(&tb, NULL);
    destroy_htable(hash_table, hash_size, pool);
    gettimeofday(&ta, NULL);
    usecs = ta.tv_sec*1000000 + ta.tv_usec - tb.tv_sec*1000000 - tb.tv_usec;
    secs = usecs/1000000;
    usecs %= 1000000;
    msecs = usecs/1000;
    usecs %= 1000;
    printf("Destruction time: %lu seconds %lu msecs %lu usecs\n", secs, msecs, usecs);

    return 0;
}
