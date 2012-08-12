#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "btree.h"

#ifdef DEBUG
#define BTREE_DEBUG(x, ...) fprintf(stderr, "%s.%d:" x, __func__, __LINE__, ##__VA_ARGS__)
#else
#define BTREE_DEBUG(x, ...) do {} while (0)
#endif

static void
__btree_move(struct btree_root *root,
        struct btree_node *n,
        unsigned int to, unsigned int from, long count)
{
    const typeof(root->size) sz = root->size;
    char *d = n->data;

    BTREE_ASSERT(count >= 0, "negative count");

    if (to == from || 0 == count)
        return;

    if (to > from) {
        from += count - 1;
        to += count - 1;
        while (count--) {
            root->assign(d+to*sz, d+from*sz);
            --to, --from;
        }
    } else {
        while (count--) {
            root->assign(d+to*sz, d+from*sz);
            ++to, ++from;
        }
    }
}

static bool
__btree_search(struct btree_root *root,
        void *key, struct btree_node **node, unsigned int *pos)
{
    cmp_result_t res;
    struct btree_node *n;
    char *d;
    const typeof(root->size) sz = root->size;
    unsigned int i;

    n = root->root;
    d = n->data;
  fln: /* find leaf node */
    for (i=0;i<n->valid;++i) {
        res = root->cmp(key, d+i*sz);
        BTREE_DEBUG("i %d, (key=%u) %7s (node->data[%d]=%u)\n",
                i, *(unsigned int *)key, stringify_cmp_result(res), i, *(unsigned int *)(d+i*sz));

        if (cmp_result_equal == res) {
            *pos = i;
            *node = n;

            return true;
        } else if (cmp_result_less == res) {
            BTREE_DEBUG("node->child[%d]=%p\n", i, n->child[i]);

            if (BTREE_EMPTY_BRANCH == n->child[i]) {
                *pos = i;
                *node = n;
                return false;
            }

            n = n->child[i];
            d = n->data;

            goto fln;
        }
    }

    BTREE_DEBUG("node->child[%d]=%p\n", i, n->child[i]);
    if (BTREE_EMPTY_BRANCH == n->child[i]) {
        *pos = i;
        *node = n;
        return false;
    }

    n = n->child[i];
    d = n->data;

    goto fln;
}

btree_add_result_t
btree_add(struct btree_root *root, void *data)
{
    struct btree_node *n, *nn = BTREE_EMPTY_BRANCH;
    const typeof(root->size) sz = root->size;
    cmp_result_t res;
    unsigned int i, j;
    char *d;

    if (btree_empty_root(root)) {
        n = root->zalloc(root);
        if (BTREE_EMPTY_BRANCH == n)
            return BTREE_ADD_ALLOC;

        n->valid = 1;
        root->assign(n->data, data);

        root->root = n;

        return BTREE_ADD_SUCCESS;
    }

    if (__btree_search(root, data, &n, &i))
        return BTREE_ADD_DUPLICATE;

    d = n->data;

  add:
    BTREE_DEBUG("add %u\n", *(unsigned int *)data);
    if (n->valid < BTREE_ORDER-1) {
        i = n->valid;
        ++n->valid;

        for (j=0;j<i;++j) {
            res = root->cmp(data, d+j*sz);
            if (cmp_result_less == res) {
                __btree_move(root, n, j+1, j, i-j);

                break;
            }
        }

        if (j != i && BTREE_EMPTY_BRANCH == nn) {
            if (BTREE_EMPTY_BRANCH != n->child[j+1])
                BTREE_DEBUG("i %d, j %d, nn %p, n->child[j+1] %p\n", i, j, nn, n->child[j+1]);
            BTREE_ASSERT(BTREE_EMPTY_BRANCH == n->child[j+1], "inserting key with right child while old key didn't have one");
        }


        root->assign(d+j*sz, data);
        if (BTREE_EMPTY_BRANCH != nn) {
            BTREE_DEBUG("i %d, j %d, nn %p, n->child[j+1] %p, nn->data[0] %u\n",
                    i, j, nn, n->child[j+1], *(unsigned int *)nn->data);
            if (BTREE_ORDER == j+1)
                BTREE_ASSERT(BTREE_EMPTY_BRANCH == n->child[j+1], "appending key in the end with right child while last child is present");
            else
                memmove(&n->child[j+2], &n->child[j+1], (i-j)*sizeof(struct btree_node *));
            n->child[j+1] = nn;
            nn->parent = n;
        }

        return BTREE_ADD_SUCCESS;
    }

    /* not enough space to store single entry
       split the node and push new median entry(possibly with right child) up
     */
    {
        struct btree_node *new;
        unsigned int pos;

        new = root->zalloc(root);
        if (BTREE_EMPTY_BRANCH == new)
            return BTREE_ADD_ALLOC;

        pos = BTREE_ORDER-1;
        for (i=0;i<BTREE_ORDER-1;++i) {
            res = root->cmp(data, d+i*sz);
            if (cmp_result_less == res) {
                pos = i;
                break;
            }
        }

        i = BTREE_ORDER/2;
        if (pos < BTREE_ORDER/2) {
            for (j=0;j<BTREE_ORDER-BTREE_ORDER/2;++j,++i) {
                BTREE_DEBUG("l.1: i %d, j %d, nn %p\n", i, j, nn);
                if (BTREE_EMPTY_BRANCH == n->child[i])
                    break;

                new->child[j] = n->child[i];
                new->child[j]->parent = new;
                n->child[i] = BTREE_EMPTY_BRANCH;
            }

            if (BTREE_EMPTY_BRANCH != nn) {
                if (BTREE_ORDER/2 == pos+1)
                    BTREE_ASSERT(BTREE_EMPTY_BRANCH == n->child[pos+1], "no place for the new child");
                else
                    memmove(&n->child[pos+2], &n->child[pos+1], (BTREE_ORDER/2-pos)*sizeof(struct btree_node *));

                n->child[pos+1] = nn;
                nn->parent = n;
            }
        } else {
            unsigned int k;
            ++i;

            for (k=i,j=0;j<BTREE_ORDER-BTREE_ORDER/2;++j,++k) {
                BTREE_DEBUG("d.2: i %d, j %d, k %d, nn %p\n", i, j, k, nn);
                if (k == pos+1) {
                    new->child[j] = nn;
                    if (BTREE_EMPTY_BRANCH != nn)
                        nn->parent = new;
                    else {
                        BTREE_ASSERT(BTREE_EMPTY_BRANCH == n->child[i], "empty child in the middle");
                        break;
                    }

                    nn = BTREE_EMPTY_BRANCH;
                } else {
                    if (BTREE_EMPTY_BRANCH == n->child[i]) {
                        BTREE_ASSERT(BTREE_EMPTY_BRANCH == nn, "new child is orphaned");
                        break;
                    }

                    new->child[j] = n->child[i];
                    new->child[j]->parent = new;
                    n->child[i] = BTREE_EMPTY_BRANCH;
                    ++i;
                }
            }
        }

        i = BTREE_ORDER/2;
        if (pos <= BTREE_ORDER/2) {
            new->valid += BTREE_ORDER-BTREE_ORDER/2-1;
            n->valid -= BTREE_ORDER-BTREE_ORDER/2-1;
            for (j=0;j<BTREE_ORDER-BTREE_ORDER/2-1;++j,++i) {
                BTREE_DEBUG("d.1: i %d, j %d, d %u\n", i, j, *(unsigned int *)(d+i*sz));
                root->assign((char *)new->data+j*sz, d+i*sz);
            }
        } else {
            unsigned int k;
            ++i;

            new->valid += BTREE_ORDER-BTREE_ORDER/2-1;
            if (pos < BTREE_ORDER-BTREE_ORDER/2-1)
                n->valid -= BTREE_ORDER-BTREE_ORDER/2-2;
            else
                n->valid -= BTREE_ORDER-BTREE_ORDER/2-1;
            for (k=i,j=0;j<BTREE_ORDER-BTREE_ORDER/2-1;++j,++k) {
                BTREE_DEBUG("d.2: i %d, j %d, k %d, d %u\n", i, j, k, *(unsigned int *)(i == pos?data:(d+i*sz)));
                if (k == pos)
                    root->assign((char *)new->data+j*sz, data);
                else
                    root->assign((char *)new->data+j*sz, d+i*sz), ++i;
            }
        }

        if (pos < BTREE_ORDER/2) {
            __btree_move(root, n, pos+1, pos, BTREE_ORDER/2-pos);
            root->assign(d+pos*sz, data);
        }

        if (BTREE_ORDER/2 != pos)
            data = d+(BTREE_ORDER/2)*sz;

        if (BTREE_EMPTY_BRANCH != n->parent) {
            BTREE_DEBUG("n->p->data[0] %u\n", *(unsigned int *)n->parent->data);
            n = n->parent;
            d = n->data;
            nn = new;
            goto add;
        } else {
            struct btree_node *nr;

            nr = root->zalloc(root);
            if (BTREE_EMPTY_BRANCH == nr)
                return BTREE_ADD_ALLOC;

            nr->child[0] = n;
            n->parent = nr;
            nr->child[1] = new;
            new->parent = nr;

            nr->valid = 1;
            root->assign(nr->data, data);

            root->root = nr;
        }
    }

    return BTREE_ADD_SUCCESS;
}

bool
btree_remove(struct btree_root *root, void *data)
{
    struct btree_node *n;
    const typeof(root->size) sz = root->size;
    cmp_result_t res;
    unsigned int i;
    unsigned int pos;
    char *d;

    if (btree_empty_root(root))
        return false;

    if (!__btree_search(root, data, &n, &pos))
        return false;

    if (BTREE_EMPTY_BRANCH == n->child[0]) {
      leaf:
        i = n->valid - 1;

        if (0 == i) { /* last entry in the node */
            struct btree_node *p = n->parent;

            if (BTREE_EMPTY_BRANCH == p)
                root->root = BTREE_EMPTY_BRANCH;
            else {
                for (i=0;i<BTREE_ORDER;++i)
                    if (p->child[i] == n) {
                        p->child[i] = BTREE_EMPTY_BRANCH;
                        break;
                    }

                BTREE_ASSERT(i < BTREE_ORDER, "can not find self in parent node");

                if (BTREE_ORDER-1 != i)
                    BTREE_ASSERT(BTREE_EMPTY_BRANCH == p->child[i+1], "deleting child in the middle");
            }

            root->free(n);
        } else {
            --n->valid;

            if (pos != i)
                __btree_move(root, n, pos, pos+1, i-pos);

            BTREE_DEBUG("node population: %u\n", i);

          rebalance:
            if (i < BTREE_ORDER/2) { /* node is half full(or half empty?) */
                struct btree_node *p = n->parent;
                struct btree_node *nn;
                unsigned int j;
                unsigned int l = BTREE_ORDER-1, r = BTREE_ORDER-1;

                if (BTREE_EMPTY_BRANCH == p) {
                    if (0 == i) {
                        root->root = n->child[0];
                        if (BTREE_EMPTY_BRANCH != n->child[0])
                            n->child[0]->parent = BTREE_EMPTY_BRANCH;
                        BTREE_ASSERT(BTREE_EMPTY_BRANCH == n->child[1], "empty root with more than one child");

                        root->free(n);
                    }

                    return true;
                }

                for (j=0;j<BTREE_ORDER;++j)
                    if (p->child[j] == n)
                        break;

                BTREE_ASSERT(j < BTREE_ORDER, "can not find self in parent node");

                if (BTREE_ORDER-1 != j && BTREE_EMPTY_BRANCH != p->child[j+1]) { /* pop element from right sibling? */
                    nn = p->child[j+1];
                    r = nn->valid;

                    BTREE_DEBUG("right sibling(with nn->data[0]=%u) population: %u\n", *(unsigned int *)nn->data, r);

                    if (r > BTREE_ORDER/2) {
                        BTREE_DEBUG("divisor(%u)=%u\n", j, *(unsigned int *)(p->data+j*sz));

                        ++n->valid;
                        root->assign(n->data+i*sz, p->data+j*sz);
                        root->assign(p->data+j*sz, nn->data);
                        --nn->valid;
                        __btree_move(root, nn, 0, 1, r-1);

                        if (BTREE_EMPTY_BRANCH != nn->child[0]) {
                            BTREE_ASSERT(BTREE_EMPTY_BRANCH != n->child[i], "gap in children nodes");
                            ++i;
                            n->child[i] = nn->child[0];
                            n->child[i]->parent = n;
                            for (j=1;j<BTREE_ORDER;++j)
                                if (BTREE_EMPTY_BRANCH == nn->child[j])
                                    break;
                            --j;
                            if (j)
                                memmove(&nn->child[0], &nn->child[1], j*sizeof(struct btree_node *));
                            nn->child[j] = BTREE_EMPTY_BRANCH;
                        }

                        return true;
                    }
                }

                if (j > 0) { /* pop element from left sibling? */
                    BTREE_ASSERT(BTREE_EMPTY_BRANCH != p->child[j-1], "no child on the left");

                    nn = p->child[j-1];
                    l = nn->valid;

                    BTREE_DEBUG("left sibling(with nn->data[0]=%u) population: %u\n", *(unsigned int *)nn->data, l);

                    if (l > BTREE_ORDER/2) {
                        BTREE_DEBUG("divisor=%u\n", *(unsigned int *)(p->data+(j-1)*sz));

                        ++n->valid;
                        __btree_move(root, n, 1, 0, i);
                        root->assign(n->data, p->data+(j-1)*sz);
                        root->assign(p->data+(j-1)*sz, nn->data+(l-1)*sz);
                        --nn->valid;

                        if (BTREE_EMPTY_BRANCH != nn->child[l]) {
                            for (j=1;j<BTREE_ORDER;++j)
                                if (BTREE_EMPTY_BRANCH == n->child[j])
                                    break;
                            if (j-1)
                                memmove(&n->child[1], &n->child[0], j*sizeof(struct btree_node *));

                            n->child[0] = nn->child[l];
                            n->child[0]->parent = n;

                            nn->child[l] = BTREE_EMPTY_BRANCH;
                        }

                        return true;
                    }
                }

                /* merge with right or left */
                if (i+r+1 <= BTREE_ORDER-1) { /* with right */
                    unsigned int k, pi;

                    BTREE_DEBUG("i %d, r %d\n", i, r);

                    nn = p->child[j+1];

                    ++n->valid;
                    root->assign(n->data+i*sz, p->data+j*sz);
                    for (k=j+2;k<BTREE_ORDER;++k)
                        if (BTREE_EMPTY_BRANCH == p->child[k])
                            break;
                    --k;
                    BTREE_DEBUG("1: k %d, j %d\n", k, j);
                    if (j != k-1)
                        memmove(&p->child[j+1], &p->child[j+2], (k-j)*sizeof(struct btree_node *));
                    p->child[k] = BTREE_EMPTY_BRANCH;
                    k = p->valid - 1;
                    pi = k;
                    BTREE_DEBUG("2: k %d, j %d\n", k, j);
                    --p->valid;
                    __btree_move(root, p, j, j+1, k-j);

                    for (k=0,++i;k<r;++k,++i) {
                        ++n->valid;
                        root->assign(n->data+i*sz, nn->data+k*sz);
                    }

                    if (BTREE_EMPTY_BRANCH != nn->child[0]) {
                        for (k=0,r=0;k<BTREE_ORDER;++k) {
                            if (BTREE_EMPTY_BRANCH != n->child[k])
                                continue;

                            n->child[k] = nn->child[r];
                            n->child[k]->parent = n;
                            ++r;

                            if (BTREE_EMPTY_BRANCH == nn->child[r])
                                break;
                        }

                        BTREE_DEBUG("k %d, r %d, nn->child[r] %p\n", k, r, nn->child[r]);
                        if (r < BTREE_ORDER)
                            BTREE_ASSERT(BTREE_EMPTY_BRANCH == nn->child[r], "not enough space to store children of right node");
                    }

                    root->free(nn);

                    if (pi < BTREE_ORDER/2) { /* rebalance parent */
                        n = p;
                        i = pi;
                        goto rebalance;
                    }

                    return true;
                } else if (i+l+1 <= BTREE_ORDER-1) { /* with left */
                    unsigned int k, pi;

                    BTREE_DEBUG("i %d, l %d\n", i, l);

                    nn = p->child[j-1];

                    ++nn->valid;
                    root->assign(nn->data+l*sz, p->data+(j-1)*sz);
                    for (k=j;k<BTREE_ORDER;++k)
                        if (BTREE_EMPTY_BRANCH == p->child[k])
                            break;
                    BTREE_DEBUG("3: k %d, j %d\n", k, j);
                    if (j != k-1)
                        memmove(&p->child[j], &p->child[j+1], (k-j)*sizeof(struct btree_node *));
                    p->child[k-1] = BTREE_EMPTY_BRANCH;
                    k = p->valid - 1;
                    pi = k;
                    BTREE_DEBUG("4: k %d, j %d\n", k, j);
                    --p->valid;
                    if (k > j)
                        __btree_move(root, p, j-1, j, k-j);

                    for (k=0,++l;k<i;++k,++l) {
                        ++nn->valid;
                        root->assign(nn->data+l*sz, n->data+k*sz);
                    }

                    if (BTREE_EMPTY_BRANCH != n->child[0]) {
                        for (k=0,l=0;l<BTREE_ORDER;++l) {
                            if (BTREE_EMPTY_BRANCH != nn->child[l])
                                continue;

                            nn->child[l] = n->child[k];
                            nn->child[l]->parent = nn;
                            ++k;

                            if (BTREE_EMPTY_BRANCH == n->child[k])
                                break;
                        }

                        BTREE_DEBUG("l %d, k %d, n->child[k] %p\n", l, k, n->child[k]);
                        if (k < BTREE_ORDER)
                            BTREE_ASSERT(BTREE_EMPTY_BRANCH == n->child[k], "not enough space to store children into left node");
                    }

                    root->free(n);

                    if (pi < BTREE_ORDER/2) { /* rebalance parent */
                        n = p;
                        i = pi;
                        goto rebalance;
                    }

                    return true;
                }

                /* fuse between right and left ? */
                BTREE_ASSERT(j > 0 && BTREE_EMPTY_BRANCH != p->child[j+1], "non-trivial rebalancing");
                BTREE_ASSERT(0, "undefined behavior");
            }
        }
    } else {
        struct btree_node *nn = n->child[pos]; /* FIXME: go to n->child[pos+1] if exists and it's more balanced than n->child[pos] */

        BTREE_ASSERT(BTREE_EMPTY_BRANCH != nn, "no child on the left");

        while (BTREE_EMPTY_BRANCH != nn->child[0]) {
            for (i=0;i<BTREE_ORDER;++i)
                if (BTREE_EMPTY_BRANCH == nn->child[i]) {
                    nn = nn->child[i-1];
                    break;
                }

            if (BTREE_ORDER == i)
                nn = nn->child[i-1];
        }

        BTREE_DEBUG("target leaf with nn->data[0]=%u\n", *(unsigned int *)nn->data);

        i = nn->valid - 1;

        BTREE_DEBUG("moving %u(%u) to %u(%u)\n", *(unsigned int *)(nn->data+i*sz), i, *(unsigned int *)(n->data+pos*sz), pos);

        root->assign(n->data+pos*sz, nn->data+i*sz);

        pos = i;
        n = nn;

        goto leaf;
    }

    return true;
}

void
__btree_traverse_prefix(struct btree_node *root,
        btree_traverse_cbk_t cbk,
        void *data)
{
    unsigned int i;

    if (BTREE_EMPTY_BRANCH == root)
        return;

    cbk(root, data);

    for (i=0;i<BTREE_ORDER;++i)
        __btree_traverse_prefix(root->child[i], cbk, data);
}

void
__btree_traverse_postfix(struct btree_node *root,
        btree_traverse_cbk_t cbk,
        void *data)
{
    unsigned int i;

    if (BTREE_EMPTY_BRANCH == root)
        return;

    for (i=0;i<BTREE_ORDER;++i)
        __btree_traverse_prefix(root->child[i], cbk, data);

    cbk(root, data);
}

bool
btree_search(struct btree_root *root,
        void *key, void **data)
{
    struct btree_node *n;
    unsigned int pos;

    if (btree_empty_root(root))
        return false;

    if (!__btree_search(root, key, &n, &pos))
        return false;

    *data = (char *)n->data + pos*root->size;

    return true;
}

#ifdef BTREE_MAIN
#include <sys/time.h>
#include <getopt.h>

#include "dot.h"
#include "sllist.h"

struct btree_el {
    unsigned int n;
    unsigned int i;
};

static cmp_result_t
btree_uint_cmp(void *a, void *b)
{
    struct btree_el *_a = a, *_b = b;
    cmp_result_t res = (cmp_result_t)int_sign(_a->n - _b->n);

    if (cmp_result_equal == res)
        return (cmp_result_t)int_sign(_a->i - _b->i);

    return res;
}

static void
btree_uint_assign(void *a, void *b)
{
    *(struct btree_el *)a = *(struct btree_el *)b;
}

static struct btree_node *
btree_node_zalloc(struct btree_root *root)
{
    return calloc(1, sizeof(struct btree_node)+root->total);
}

static void
btree_node_free(struct btree_node *node)
{
    free(node);
}

struct btree_node_dot_info {
    FILE *out;
};

void
__dump_btree_graph(struct btree_node *root,
        struct btree_node_dot_info *info)
{
    unsigned int i;
    struct btree_el *d;
    char name[100];
    char *entries[BTREE_ORDER];
    char __entries[BTREE_ORDER][16];

    for (i=0;i<BTREE_ORDER-1;++i) {
        entries[i] = __entries[i];
        d = (struct btree_el *)((char *)root->data + i*sizeof(struct btree_el));

        memset(entries[i], 0x0, sizeof(entries[0]));

        if (i >= root->valid)
            sprintf(entries[i], "%s", "-");
        else
            sprintf(entries[i], "%u.%u", d->n, d->i);
    }

    dot_dump_btree_node(info->out, "btree_node", (unsigned long)root, (const char **)entries, BTREE_ORDER);

    sprintf(name, "btree_node_%u", root);

    for (i=0;i<BTREE_ORDER;++i)
        if (BTREE_EMPTY_BRANCH != root->child[i])
            dot_dump_link_table_to_node(info->out, name, (unsigned long)i, "btree_node", (unsigned long)root->child[i]);
}

void
dump_btree_graph(const char *graph,
        struct btree_root *root)
{
    struct btree_node_dot_info info = {
        .out = fopen(graph, "w+"),
    };

    if (!info.out)
        return;

    dot_dump_begin(info.out, "btree", dot_graph_direction_top_to_bottom);
    btree_traverse(btree_traverse_type_prefix,
            root, (btree_traverse_cbk_t)__dump_btree_graph, (void *)&info);
    dot_dump_end(info.out);

    fclose(info.out);
}

void
dump_btree_node(struct btree_node *node, void *data)
{
    unsigned int i;
    struct btree_el *d;

    for (i=0;i<node->valid;++i) {
        d = (struct btree_el *)((char *)node->data + i*sizeof(struct btree_el));
        printf("%u.%u ", d->n, d->i);
    }
}

static void
construct_btree(struct btree_el *array, int count, struct btree_root *root)
{
    unsigned int i;

    btree_init_root(root, sizeof(struct btree_el),
            btree_uint_cmp, btree_uint_assign, btree_node_zalloc, btree_node_free);

    for (i=0;i<count;++i) {
        btree_add_result_t res;

        //printf("Inserting %u\n", array[i]);
        res = btree_add(root, &array[i]);
        if (BTREE_ADD_SUCCESS != res) {
            fprintf(stderr, "Unable to insert %u.%u: %d\n", array[i].n, array[i].i, res);
            break;
        }
    }
}

unsigned int
destroy_btree(struct btree_root *root)
{
    unsigned int removed = 0;

    while (!btree_empty_root(root)) {
        if (!btree_remove(root, root->root->data)) {
            fprintf(stderr, "Unable remove element\n");
            break;
        }

        ++removed;
    }

    return removed;
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

    unsigned int removed;

    unsigned int *nums;
    struct btree_el *array;
    struct btree_root btree_root;

    int i;

    int opt;
    const struct option options[] = {
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
            case 0:
                break;
            default:
                return usage(argv[0]);
        }
    }

    if (generate_array(&nums, &count, input_data) == -1)
        return 1;

    printf("%d elements\n", count);
    if (dump)
        print_array(nums, count);

    array = malloc(count*sizeof(struct btree_el));
    if (!array) {
        fprintf(stderr, "Unable to allocate %u bytes for btree elements\n", count*sizeof(struct btree_el));
        free(nums);
        return 1;
    }

    for (i=0;i<count;++i) {
        array[i].n = nums[i];
        array[i].i = i;
    }
    free(nums);

    gettimeofday(&tb, NULL);
    construct_btree(array, count, &btree_root);
    gettimeofday(&ta, NULL);
    usecs = ta.tv_sec*1000000 + ta.tv_usec - tb.tv_sec*1000000 - tb.tv_usec;
    secs = usecs/1000000;
    usecs %= 1000000;
    msecs = usecs/1000;
    usecs %= 1000;
    printf("Construct time: %lu seconds %lu msecs %lu usecs\n", secs, msecs, usecs);

    if (dump) {
        printf("Prefix walk:\n");
        btree_traverse(btree_traverse_type_prefix,
                &btree_root, dump_btree_node, NULL);
        printf("\n");
    }

    {
        struct btree_el *data;
        unsigned int i;
        bool found;

        usecs = 0;
        for (i=0;i<count;++i) {
            gettimeofday(&tb, NULL);
            found = btree_search(&btree_root, &array[i], (void **)&data);
            gettimeofday(&ta, NULL);

            usecs += ta.tv_sec*1000000 + ta.tv_usec - tb.tv_sec*1000000 - tb.tv_usec;

            if (!found) {
                fprintf(stderr, "Element %u(%u) was not found\n", i, array[i]);
                goto out;
            }

            if (data->n != array[i].n || data->i != array[i].i) {
                fprintf(stderr, "Found element %u.%u does not match to the search criteria %u.%u\n",
                        data->n, data->i, array[i].i, array[i].i);
                goto out;
            }
        }

        secs = usecs/1000000;
        usecs %= 1000000;
        msecs = usecs/1000;
        usecs %= 1000;

        printf("All elements of array were successfully found, avg time: %lu seconds %lu msecs %lu usecs\n", secs, msecs, usecs);
    }

    {
        struct btree_el *data;
        unsigned int i;
        unsigned int c = count/2;

        for (i=0;i<c;++i) {
            if (!btree_search(&btree_root, &array[i], (void **)&data)) {
                fprintf(stderr, "Element %u(%u.%u) was not found\n", i, array[i].n, array[i].i);
                goto out;
            }

            if (data->n != array[i].n || data->i != array[i].i) {
                fprintf(stderr, "Found element %u.%u does not match to the search criteria %u.%u\n",
                        data->n, data->i, array[i].i, array[i].i);
                goto out;
            }

            //printf("Removing %u.%u\n", data->n, data->i);

            if (!btree_remove(&btree_root, (void *)data)) {
                fprintf(stderr, "Unable to remove element %u.u\n", data->n, data->i);
                goto out;
            }
        }

        printf("Removed %d tree nodes\n", c);

        for (i=c;i<count;++i) {
            if (!btree_search(&btree_root, &array[i], (void **)&data)) {
                fprintf(stderr, "Element %u(%u.%u) was not found\n", i, array[i].n, array[i].i);
                goto out;
            }

            if (data->n != array[i].n || data->i != array[i].i) {
                fprintf(stderr, "Found element %u.%u does not match to the search criteria %u.%u\n",
                        data->n, data->i, array[i].i, array[i].i);
                goto out;
            }
        }

        printf("Rest elements of array were successfully found\n");
    }

  out:
    if (graph)
        dump_btree_graph(graph, &btree_root);

    free(array);

    gettimeofday(&tb, NULL);
    removed = destroy_btree(&btree_root);
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
