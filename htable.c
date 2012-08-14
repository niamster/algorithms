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

typedef unsigned int (*hash_function_t)(const char *, unsigned int len, unsigned int bits);

struct hash_node {
#if defined(HTABLE_LIST)
    struct sllist list;
#elif defined(HTABLE_TREE)
    struct binary_tree_node tree;
#endif
    /* typeof(((struct key_value *)0)->key) key[sizeof(((struct key_value *)0)->key)]; */
    unsigned char key[KEY_LEN];
    struct key_value *data;
};

struct hash_table {
    hash_function_t hash_function;
    unsigned int bits;
#if defined(HTABLE_LIST)
    struct sllist *table;
#elif defined(HTABLE_TREE)
    struct binary_tree_node *table;
#else
    void *table;
    unsigned int node_size;
#endif
    unsigned int entries;
};

#if defined(HTABLE_TREE)
cmp_result_t
binary_tree_str_key_cmp(struct binary_tree_node *one,
                        struct binary_tree_node *two)
{
    struct hash_node *_one = container_of(one, struct hash_node, tree);
    struct hash_node *_two = container_of(two, struct hash_node, tree);

    return (cmp_result_t)int_sign(strcmp(_two->key, _one->key));
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
__print_htable_binary_tree_entries(struct binary_tree_node *root,
                                   void *__unused__)
{
    struct hash_node *node = container_of(root, struct hash_node, tree);
    printf("    '%s' => '%s'\n", node->data->key, node->data->value);
}

void
print_htable_entries(struct binary_tree_node *root)
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
print_htable(struct hash_table *hash_table)
{
    printf("htable of %d entries\n", hash_table->entries);

    int i;
    for (i=0;i<hash_table->entries;++i) {
        printf("Level %d\n", i);

        print_htable_entries(&hash_table->table[i]);
    }
}

#ifdef HTABLE_TREE
struct binary_tree_node_dot_info {
    FILE *out;
    int id;
};

void
__dump_htable_binary_tree_graph(struct binary_tree_node *root,
                                struct binary_tree_node_dot_info *info)
{
    struct hash_node *node = container_of(root, struct hash_node, tree);

    dot_dump_node(info->out, "binary_tree_node", (unsigned long)root, node->data->key);

    if (root->left != BINARY_TREE_EMPTY_BRANCH) {
        dot_dump_link_node_to_node(info->out, "binary_tree_node", (unsigned long)root, "binary_tree_node", (unsigned long)root->left);
    } else {
        dot_dump_shape_colored(info->out, "NULL", info->id, "", "red", "black", "black", "circle");
        dot_dump_link_node_to_node(info->out, "binary_tree_node", (unsigned long)root, "NULL", (unsigned long)info->id);
        ++info->id;
    }

    if (root->right != BINARY_TREE_EMPTY_BRANCH) {
        dot_dump_link_node_to_node(info->out, "binary_tree_node", (unsigned long)root, "binary_tree_node", (unsigned long)root->right);
    } else {
        dot_dump_shape_colored(info->out, "NULL", info->id, "", "red", "black", "black", "circle");
        dot_dump_link_node_to_node(info->out, "binary_tree_node", (unsigned long)root, "NULL", (unsigned long)info->id);
        ++info->id;
    }
}
#endif

void
dump_htable_graph(const char *graph,
        struct hash_table *hash_table)
{
    int i;
    FILE *out = fopen(graph, "w+");
#if defined(HTABLE_TREE)
    struct binary_tree_node_dot_info info = {
        .out = out,
        .id = 0
    };
#endif

    if (!out)
        return;

    dot_dump_begin(out, "htable", dot_graph_direction_left_to_right);
    dot_dump_table(out, "htable", hash_table->entries);
    for (i=0;i<hash_table->entries;++i) {
#if defined(HTABLE_LIST)
        if (!sllist_empty(&hash_table->table[i])) {
            dot_dump_sllist(out, "list", i, &hash_table->table[i], struct hash_node, list, data->key);
            dot_dump_link_table_to_sllist_head(out, "htable", i, "list", i);
        }
#elif defined(HTABLE_TREE)
        binary_tree_traverse(binary_tree_traverse_type_prefix,
                             &hash_table->table[i], (binary_tree_traverse_cbk_t)__dump_htable_binary_tree_graph, (void *)&info);
        dot_dump_link_table_to_node(out, "htable", i, "binary_tree_node", (unsigned long)binary_tree_node(&hash_table->table[i]));
#endif
    }
    dot_dump_end(out);

    fclose(out);
}

#if defined(HTABLE_TREE)
static void
htable_binary_tree_weight(struct binary_tree_node *node, void *data)
{
    int *weight = (int *)data;
    ++weight;
}
#endif

void
print_htable_summary(struct hash_table *hash_table,
        int count)
{
    printf("htable of %d entries\n", hash_table->entries);

    int i;
    for (i=0;i<hash_table->entries;++i) {
        int level_count = 0;

#if defined(HTABLE_LIST)
        struct sllist *e;
        sllist_for_each(&hash_table->table[i], e) {
            ++level_count;
        }
#elif defined(HTABLE_TREE)
        binary_tree_traverse(binary_tree_traverse_type_infix, &hash_table->table[i], htable_binary_tree_weight, (void *)&level_count);
#endif

        printf("Level %5d: %5d entries(%.2f%%)\n", i, level_count, ((float)level_count/(float)count)*100);
    }
}

int
construct_htable(struct key_value *data,
                 int count,
                 struct hash_table *hash_table,
                 int hash_size,
                 hash_function_t hash_function,
                 struct hash_node **pool)
{
    int i;
    struct hash_node *nodes;

    hash_size = roundup_pow_of_two(hash_size);

#if defined(HTABLE_LIST)
    if (!(hash_table->table = malloc(hash_size*sizeof(struct sllist)))) {
        fprintf(stderr, "Error error allocating %d bytes: %s", hash_size*sizeof(struct sllist), strerror(errno));
        return -1;
    }
#elif defined(HTABLE_TREE)
    if (!(hash_table->table = malloc(hash_size*sizeof(struct binary_tree_node)))) {
        fprintf(stderr, "Error error allocating %d bytes: %s", hash_size*sizeof(struct binary_tree_node), strerror(errno));
        return -1;
    }
#endif

    hash_table->entries = hash_size;
    hash_table->bits = ilog2(hash_size);
    hash_table->hash_function = hash_function;

    for (i=0;i<hash_size;++i) {
#if defined(HTABLE_LIST)
        sllist_init(&hash_table->table[i]);
#elif defined(HTABLE_TREE)
        __binary_tree_init_root(&hash_table->table[i]);
#endif
    }

    if (!(*pool = malloc(count*sizeof(struct hash_node)))) {
        fprintf(stderr, "Error error allocating %d bytes: %s", count*sizeof(struct hash_node), strerror(errno));
        return -1;
    }

    nodes = *pool;

    for (i=0;i<count;++i) {
        int hash = hash_function(data[i].key, data[i].key?strlen(data[i].key):0, hash_table->bits);
        if (hash >= hash_size) { /* sanity checks */
            fprintf(stderr, "Invalid hash: %d for %s, skipping element\n", hash, data[i].key);
            continue;
        }

        nodes[i].data = &data[i];
        if (data[i].key)
            memcpy(nodes[i].key, data[i].key, KEY_LEN);
#if defined(HTABLE_LIST)
        sllist_add(&hash_table->table[hash], &nodes[i].list);
#elif defined(HTABLE_TREE)
        binary_tree_init_node(&nodes[i].tree);
        binary_tree_add(&hash_table->table[hash], &nodes[i].tree, binary_tree_str_key_cmp);
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
destroy_htable_entries(struct binary_tree_node *root)
{
    struct binary_tree_node *r, *left, *right;
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
destroy_htable(struct hash_table *hash_table,
               struct hash_node *pool)
{
    int i;
    for (i=0;i<hash_table->entries;++i)
        destroy_htable_entries(&hash_table->table[i]);

    free(pool);
    free(hash_table->table);
}

#if defined(HTABLE_TREE)
cmp_result_t
binary_tree_str_key_match(struct binary_tree_node *node,
                          void *key)
{
    struct hash_node *_node = container_of(node, struct hash_node, tree);

    if (!key)
        return _node->data->key?cmp_result_greater:cmp_result_equal;

    return (cmp_result_t)int_sign(strcmp(key, _node->key));
}
#endif

void
search_htable(struct hash_table *hash_table,
              const unsigned char *key,
              struct sllist *result,
              int limit)
{
#if defined(HTABLE_LIST)
    struct sllist *e;
#endif

    int hash = hash_table->hash_function(key, key?strlen(key):0, hash_table->bits);
    if (hash >= hash_table->entries) { /* sanity checks */
        fprintf(stderr, "Invalid hash: %d for %s\n", hash, key);
        return;
    }

#if defined(HTABLE_LIST)
    sllist_for_each (&hash_table->table[hash], e) {
        struct hash_node *needle;
        struct hash_node *node = container_of(e, struct hash_node, list);

        if (!key) {
            if (node->data->key)
                continue;
        } else if (strcmp(key, node->key)) {
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
    binary_tree_search(&hash_table->table[hash], (void *)key, binary_tree_str_key_match, result, limit);
#endif
}

unsigned int
additive_hash(const char *key,
              unsigned int len,
              unsigned int bits)
{
    const char *end = key + len;
    unsigned int hash = len;

    if (!key)
        return 0;

    while (key < end)
	    hash += *key++;

	return (((hash >> bits) ^ hash) & ((1 << bits) - 1));
}

unsigned int
rotating_hash(const char *key,
              unsigned int len,
              unsigned int bits)
{
    const char *end = key + len;
    unsigned int hash = len;

    if (!key)
        return 0;

    while (key < end)
        hash = (hash<<4)^(hash>>28)^*key++;

    return (((hash >> bits) ^ hash) & ((1 << bits) - 1));
}

/* From http://www.azillionmonkeys.com/qed/hash.html */
#undef get16bits
#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__)    \
    || defined(_MSC_VER) || defined (__BORLANDC__) || defined (__TURBOC__)
#define get16bits(d) (*((const unsigned short *) (d)))
#else
#define get16bits(d) ((((unsigned int)(((const unsigned char *)(d))[1])) << 8)    \
            +(unsigned int)(((const unsigned char *)(d))[0]) )
#endif

unsigned int
super_fast_hash(const char *key,
        unsigned int len,
        unsigned int bits)
{
    unsigned int hash = len, tmp;
    int rem;

    if (!key)
        return 0;

    rem = len & 3;
    len >>= 2;

    /* Main loop */
    for (;len > 0; len--) {
        hash  += get16bits (key);
        tmp    = (get16bits (key+2) << 11) ^ hash;
        hash   = (hash << 16) ^ tmp;
        key  += 2*sizeof (unsigned short);
        hash  += hash >> 11;
    }

    /* Handle end cases */
    switch (rem) {
        case 3: hash += get16bits (key);
            hash ^= hash << 16;
            hash ^= key[sizeof (unsigned short)] << 18;
            hash += hash >> 11;
            break;
        case 2: hash += get16bits (key);
            hash ^= hash << 11;
            hash += hash >> 17;
            break;
        case 1: hash += *key;
            hash ^= hash << 10;
            hash += hash >> 1;
    }

    /* Force "avalanching" of final 127 bits */
    hash ^= hash << 3;
    hash += hash >> 5;
    hash ^= hash << 4;
    hash += hash >> 17;
    hash ^= hash << 25;
    hash += hash >> 6;

    return (((hash >> bits) ^ hash) & ((1 << bits) - 1));
}

/* From http://burtleburtle.net/bob/hash/doobs.html */
#define hashsize(n) ((unsigned int)1<<(n))
#define hashmask(n) (hashsize(n)-1)

/*
--------------------------------------------------------------------
mix -- mix 3 32-bit values reversibly.
For every delta with one or two bits set, and the deltas of all three
  high bits or all three low bits, whether the original value of a,b,c
  is almost all zero or is uniformly distributed,
* If mix() is run forward or backward, at least 32 bits in a,b,c
  have at least 1/4 probability of changing.
* If mix() is run forward, every bit of c will change between 1/3 and
  2/3 of the time.  (Well, 22/100 and 78/100 for some 2-bit deltas.)
mix() was built out of 36 single-cycle latency instructions in a
  structure that could supported 2x parallelism, like so:
      a -= b;
      a -= c; x = (c>>13);
      b -= c; a ^= x;
      b -= a; x = (a<<8);
      c -= a; b ^= x;
      c -= b; x = (b>>13);
      ...
  Unfortunately, superscalar Pentiums and Sparcs can't take advantage
  of that parallelism.  They've also turned some of those single-cycle
  latency instructions into multi-cycle latency instructions.  Still,
  this is the fastest good hash I could find.  There were about 2^^68
  to choose from.  I only looked at a billion or so.
--------------------------------------------------------------------
*/
#define mix(a,b,c)                              \
    {                                           \
        a -= b; a -= c; a ^= (c>>13);           \
        b -= c; b -= a; b ^= (a<<8);            \
        c -= a; c -= b; c ^= (b>>13);           \
        a -= b; a -= c; a ^= (c>>12);           \
        b -= c; b -= a; b ^= (a<<16);           \
        c -= a; c -= b; c ^= (b>>5);            \
        a -= b; a -= c; a ^= (c>>3);            \
        b -= c; b -= a; b ^= (a<<10);           \
        c -= a; c -= b; c ^= (b>>15);           \
    }

/*
--------------------------------------------------------------------
hash() -- hash a variable-length key into a 32-bit value
  k       : the key (the unaligned variable-length array of bytes)
  len     : the length of the key, counting by bytes
  initval : can be any 4-byte value
Returns a 32-bit value.  Every bit of the key affects every bit of
the return value.  Every 1-bit and 2-bit delta achieves avalanche.
About 6*len+35 instructions.

The best hash table sizes are powers of 2.  There is no need to do
mod a prime (mod is sooo slow!).  If you need less than 32 bits,
use a bitmask.  For example, if you need only 10 bits, do
  h = (h & hashmask(10));
In which case, the hash table should have hashsize(10) elements.

If you are hashing n strings (unsigned char **)k, do it like this:
  for (i=0, h=0; i<n; ++i) h = hash( k[i], len[i], h);

By Bob Jenkins, 1996.  bob_jenkins@burtleburtle.net.  You may use this
code any way you wish, private, educational, or commercial.  It's free.

See http://burtleburtle.net/bob/hash/evahash.html
Use for hash table lookup, or anything where one collision in 2^^32 is
acceptable.  Do NOT use for cryptographic purposes.
--------------------------------------------------------------------
*/

unsigned int
bob_jenkin_hash(const char *key,
        unsigned int len,
        unsigned int bits)
{
    unsigned int initval = 0;  /* the previous hash, or an arbitrary value */
    unsigned int a, b, c, l;

    /* Set up the internal state */
    l = len;
    a = b = 0x9e3779b9;  /* the golden ratio; an arbitrary value */
    c = initval;         /* the previous hash value */

    /*---------------------------------------- handle most of the key */
    while (l >= 12) {
        a += (key[0] +((unsigned int)key[1]<<8) +((unsigned int)key[2]<<16) +((unsigned int)key[3]<<24));
        b += (key[4] +((unsigned int)key[5]<<8) +((unsigned int)key[6]<<16) +((unsigned int)key[7]<<24));
        c += (key[8] +((unsigned int)key[9]<<8) +((unsigned int)key[10]<<16)+((unsigned int)key[11]<<24));
        mix(a,b,c);
        key += 12; l -= 12;
    }

    /*------------------------------------- handle the last 11 bytes */
    c += len;
    switch (l) {             /* all the case statements fall through */
        case 11: c+=((unsigned int)key[10]<<24);
        case 10: c+=((unsigned int)key[9]<<16);
        case 9 : c+=((unsigned int)key[8]<<8);
            /* the first byte of c is reserved for the len */
        case 8 : b+=((unsigned int)key[7]<<24);
        case 7 : b+=((unsigned int)key[6]<<16);
        case 6 : b+=((unsigned int)key[5]<<8);
        case 5 : b+=key[4];
        case 4 : a+=((unsigned int)key[3]<<24);
        case 3 : a+=((unsigned int)key[2]<<16);
        case 2 : a+=((unsigned int)key[1]<<8);
        case 1 : a+=key[0];
            /* case 0: nothing left to add */
    }
    mix(a,b,c);

    /*-------------------------------------------- report the result */
    return (((c >> bits) ^ c) & ((1 << bits) - 1));
}

unsigned int
sdbm_hash(const char *key,
        unsigned int len,
        unsigned int bits)
{
    const char *end = key + len;
    unsigned int hash = 0;

    if (!key)
        return 0;

    while (key < end)
        hash = *key++ + (hash << 6) + (hash << 16) - hash;

    return (((hash >> bits) ^ hash) & ((1 << bits) - 1));
}

/* From http://www.isthe.com/chongo/src/fnv/ */
#define FNV_32_PRIME 0x01000193

unsigned int
fnva_hash(const char *key,
        unsigned int len,
        unsigned int bits)
{
    const char *end = key + len;
    unsigned int hash = FNV_32_PRIME;

    /*
     * FNV-1a hash each octet in the buffer
     */
    while (key < end) {
        /* xor the bottom with the current octet */
        hash ^= *key++;

        /* multiply by the 32 bit FNV magic prime mod 2^32 */
#if defined(NO_FNV_GCC_OPTIMIZATION)
#warning GCC optimizations for FNV-a not used
        hash *= FNV_32_PRIME;
#else
        hash += (hash<<1) + (hash<<4) + (hash<<7) + (hash<<8) + (hash<<24);
#endif
    }

    /* return our new hash value */
    return (((hash >> bits) ^ hash) & ((1 << bits) - 1));
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
    struct hash_table hash_table;
    struct hash_node *pool;
    int count = 0;

    const char *input_data = "/dev/random";
    struct key_value *data;
    const unsigned char *key = "needle";
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
                else if (!strncmp(optarg, "sfh", 3))
                    hash_function = super_fast_hash;
                else if (!strncmp(optarg, "bob-jenkin", 10))
                    hash_function = bob_jenkin_hash;
                else if (!strncmp(optarg, "sdbm", 4))
                    hash_function = sdbm_hash;
                else if (!strncmp(optarg, "fnva", 4))
                    hash_function = fnva_hash;
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
    printf("Hash entries: %u bits: %d\n", hash_table.entries, hash_table.bits);
    usecs = ta.tv_sec*1000000 + ta.tv_usec - tb.tv_sec*1000000 - tb.tv_usec;
    secs = usecs/1000000;
    usecs %= 1000000;
    msecs = usecs/1000;
    usecs %= 1000;
    printf("Construct time: %lu seconds %lu msecs %lu usecs\n", secs, msecs, usecs);

    if (dump_htable)
        print_htable(&hash_table);
    if (dump_htable_summary)
        print_htable_summary(&hash_table, count);

    sllist_init(&values);

    gettimeofday(&tb, NULL);
    search_htable(&hash_table, key, &values, limit);
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
        dump_htable_graph(graph, &hash_table);
    }

  out:
    free(data);

    gettimeofday(&tb, NULL);
    destroy_htable(&hash_table, pool);
    gettimeofday(&ta, NULL);
    usecs = ta.tv_sec*1000000 + ta.tv_usec - tb.tv_sec*1000000 - tb.tv_usec;
    secs = usecs/1000000;
    usecs %= 1000000;
    msecs = usecs/1000;
    usecs %= 1000;
    printf("Destruction time: %lu seconds %lu msecs %lu usecs\n", secs, msecs, usecs);

    return 0;
}
