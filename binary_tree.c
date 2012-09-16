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

    if (node->right != BINARY_TREE_EMPTY_BRANCH) {
        node->right->parent = node;
#if defined(BINARY_TREE_RANDOM)
        node->weight += node->right->weight + 1;
#endif
    }

#if defined(BINARY_TREE_AVL)
    node->height = max(BINARY_TREE_AVL_NODE_HEIGHT(node->right), BINARY_TREE_AVL_NODE_HEIGHT(node->left)) + 1;
    r->height = max(BINARY_TREE_AVL_NODE_HEIGHT(r->right), BINARY_TREE_AVL_NODE_HEIGHT(r->left)) + 1;
#endif

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

    if (node->left != BINARY_TREE_EMPTY_BRANCH) {
        node->left->parent = node;
#if defined(BINARY_TREE_RANDOM)
        node->weight += node->left->weight + 1;
#endif
    }

#if defined(BINARY_TREE_AVL)
    node->height = max(BINARY_TREE_AVL_NODE_HEIGHT(node->right), BINARY_TREE_AVL_NODE_HEIGHT(node->left)) + 1;
    l->height = max(BINARY_TREE_AVL_NODE_HEIGHT(l->right), BINARY_TREE_AVL_NODE_HEIGHT(l->left)) + 1;
#endif

    l->parent = p;
    node->parent = l;
}

#if defined(BINARY_TREE_AVL)
static void
binary_tree_avl_rebalance(struct binary_tree_node *parent)
{
    struct binary_tree_node *node;
    int balance;

    while (!binary_tree_root_node(parent)) {
        parent->height = max(BINARY_TREE_AVL_NODE_HEIGHT(parent->right), BINARY_TREE_AVL_NODE_HEIGHT(parent->left)) + 1;

        balance = BINARY_TREE_AVL_NODE_BALANCE(parent);

        /* if (balance >= -1 && balance <= 1) */
        /*     break; */
        /* else */ if (balance == -2) {
            if (BINARY_TREE_AVL_NODE_BALANCE(parent->right) == 1) {
                node = parent->right->left;

                binary_tree_rotate_right(parent->right);
                binary_tree_rotate_left(parent);
            } else {
                node = parent->right;
                binary_tree_rotate_left(parent);
            }
        } else if (balance == 2) {
            if (BINARY_TREE_AVL_NODE_BALANCE(parent->left) == -1) {
                node = parent->left->right;

                binary_tree_rotate_left(parent->left);
                binary_tree_rotate_right(parent);
            } else {
                node = parent->left;
                binary_tree_rotate_right(parent);
            }
        } else
            node = parent;

        parent = node->parent;
    }
}

static inline void
binary_tree_avl_rebalance_on_insert(struct binary_tree_node *node)
{
    binary_tree_avl_rebalance(node->parent);
}

static inline void
binary_tree_avl_rebalance_on_delete(struct binary_tree_node *parent)
{
    binary_tree_avl_rebalance(parent);
}
#elif defined(BINARY_TREE_RB)
static void
binary_tree_rb_rebalance_on_insert(struct binary_tree_node *node)
{
    struct binary_tree_node *p = node->parent, *gp, *uncle;

    while (p->color == BINARY_TREE_RB_RED) {
        gp = p->parent;
        if (gp->left == p)
            uncle = gp->right;
        else
            uncle = gp->left;

        if (BINARY_TREE_RB_NODE_COLOR(uncle) == BINARY_TREE_RB_RED) {
            uncle->color = p->color = BINARY_TREE_RB_BLACK;

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
binary_tree_rb_rebalance_on_delete(struct binary_tree_node *node, struct binary_tree_node *parent)
{
	while (BINARY_TREE_RB_NODE_COLOR(node) && !binary_tree_root_node(parent)) {
        struct binary_tree_node *sibling;

		if (parent->left == node) {
			sibling = parent->right;

            if (sibling->color == BINARY_TREE_RB_RED) {
                sibling->color = BINARY_TREE_RB_BLACK;
                parent->color = BINARY_TREE_RB_RED;
				binary_tree_rotate_left(parent);
				sibling = parent->right;
			}

			if (BINARY_TREE_RB_NODE_COLOR(sibling->left) == BINARY_TREE_RB_BLACK
                    && BINARY_TREE_RB_NODE_COLOR(sibling->right) == BINARY_TREE_RB_BLACK) {
                sibling->color = BINARY_TREE_RB_RED;
				node = parent;
				parent = node->parent;
			} else {
				if (BINARY_TREE_RB_NODE_COLOR(sibling->right) == BINARY_TREE_RB_BLACK) {
                    sibling->left->color = BINARY_TREE_RB_BLACK;
                    sibling->color = BINARY_TREE_RB_RED;
                    binary_tree_rotate_right(sibling);
					sibling = parent->right;
				}
                sibling->color = parent->color;
                parent->color = BINARY_TREE_RB_BLACK;
                sibling->right->color = BINARY_TREE_RB_BLACK;
                binary_tree_rotate_left(parent);
				break;
			}
		} else {
			sibling = parent->left;

            if (sibling->color == BINARY_TREE_RB_RED) {
                sibling->color = BINARY_TREE_RB_BLACK;
                parent->color = BINARY_TREE_RB_RED;
				binary_tree_rotate_right(parent);
				sibling = parent->left;
			}

            if (BINARY_TREE_RB_NODE_COLOR(sibling->left) == BINARY_TREE_RB_BLACK
                    && BINARY_TREE_RB_NODE_COLOR(sibling->right) == BINARY_TREE_RB_BLACK) {
                sibling->color = BINARY_TREE_RB_RED;
				node = parent;
                parent = node->parent;
			} else {
				if (BINARY_TREE_RB_NODE_COLOR(sibling->left) == BINARY_TREE_RB_BLACK) {
                    sibling->right->color = BINARY_TREE_RB_BLACK;
                    sibling->color = BINARY_TREE_RB_RED;
                    binary_tree_rotate_left(sibling);
					sibling = parent->left;
				}
                sibling->color = parent->color;
                parent->color = BINARY_TREE_RB_BLACK;
                sibling->left->color = BINARY_TREE_RB_BLACK;
                binary_tree_rotate_right(parent);
				break;
			}
		}
	}

	if (node != BINARY_TREE_EMPTY_BRANCH)
        node->color = BINARY_TREE_RB_BLACK;
}
#elif defined(BINARY_TREE_RANDOM)
static void
binary_tree_random_update_weight_on_delete(struct binary_tree_node *node)
{
    while (!binary_tree_root_node(node)) {
        --node->weight;
        node = node->parent;
    }
}
#elif defined(BINARY_TREE_TREAP)
static void
binary_tree_treap_rebalance_on_insert(struct binary_tree_node *node)
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
    typeof(node->weight) weight = node->weight + 1;

    for (;;) {
        if (dst == NULL
                && get_random()%(1+r->weight) == 0)
            dst = r;

        r->weight += weight;

        res = cmp(node, r);

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
        res = cmp(node, r);
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
    if (cmp(node, root->leftmost) == cmp_result_less)
        root->leftmost = node;
    else if (cmp(node, root->rightmost) == cmp_result_greater)
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
    binary_tree_treap_rebalance_on_insert(node);
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
    struct binary_tree_search_result *result;
    int matched = 0;

    do {
        res = match(n, key);

        if (res == cmp_result_equal) {
            if (!(result = malloc(sizeof(struct binary_tree_search_result)))) {
                fprintf(stderr, "Error allocating %d bytes: %s", sizeof(struct binary_tree_search_result), strerror(errno));
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
        struct binary_tree_search_result *r = container_of(e, struct binary_tree_search_result, list);

        sllist_detach(e, p);
        free(r);
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
    binary_tree_avl_rebalance_on_delete(p);
#elif defined(BINARY_TREE_RB)
    binary_tree_rb_rebalance_on_delete(node, p);
#elif defined(BINARY_TREE_RANDOM)
    binary_tree_random_update_weight_on_delete(p);
#endif

    node->parent = node;
}

void
__binary_tree_remove(struct binary_tree_node *node)
{
    struct binary_tree_node **pp, *child, *parent, *old = node;
#if defined(BINARY_TREE_RB)
	typeof(node->color) color;
#endif

    parent = node->parent;
    pp = (parent->left == node)?&parent->left:&parent->right;

	if (node->left == BINARY_TREE_EMPTY_BRANCH)
		child = node->right;
	else if (node->right == BINARY_TREE_EMPTY_BRANCH)
		child = node->left;
	else {
#if defined(BINARY_TREE_RANDOM)
        int leftmost = 1;
        if (BINARY_TREE_RANDOM_NODE_WEIGHT(node->right) + BINARY_TREE_RANDOM_NODE_WEIGHT(node->left) == 0
                || get_random()%(BINARY_TREE_RANDOM_NODE_WEIGHT(node->right) + BINARY_TREE_RANDOM_NODE_WEIGHT(node->left)) < BINARY_TREE_RANDOM_NODE_WEIGHT(node->left))
            leftmost = 0;
#else
        const int leftmost = 1;
#endif
        if (leftmost) {
            node = node->right;
            while (node->left != BINARY_TREE_EMPTY_BRANCH)
                node = node->left;

            *pp = node;

            child = node->right;
            parent = node->parent;
#if defined(BINARY_TREE_RB)
            color = node->color;
#endif

            if (parent == old) {
                parent = node;
            } else {
                if (child != BINARY_TREE_EMPTY_BRANCH)
                    child->parent = parent;
                parent->left = child;

                node->right = old->right;
                old->right->parent = node;
            }

            node->parent = old->parent;
            node->left = old->left;
            old->left->parent = node;
        } else {
            node = node->left;
            while (node->right != BINARY_TREE_EMPTY_BRANCH)
                node = node->right;

            *pp = node;

            child = node->left;
            parent = node->parent;
#if defined(BINARY_TREE_RB)
            color = node->color;
#endif

            if (parent == old) {
                parent = node;
            } else {
                if (child != BINARY_TREE_EMPTY_BRANCH)
                    child->parent = parent;
                parent->right = child;

                node->left = old->left;
                old->left->parent = node;
            }

            node->parent = old->parent;
            node->right = old->right;
            old->right->parent = node;
        }

#if defined(BINARY_TREE_AVL)
        node->height = old->height;
#elif defined(BINARY_TREE_RB)
        node->color = old->color;
#elif defined(BINARY_TREE_RANDOM)
        node->weight = old->weight;
#elif defined(BINARY_TREE_TREAP)
        node->prio = old->prio;
#endif

		goto rebalance;
	}

#if defined(BINARY_TREE_RB)
	color = node->color;
#endif

	if (child != BINARY_TREE_EMPTY_BRANCH)
        child->parent = parent;

    *pp = child;

 rebalance:
#if defined(BINARY_TREE_AVL)
    binary_tree_avl_rebalance_on_delete(parent);
#elif defined(BINARY_TREE_RB)
	if (color == BINARY_TREE_RB_BLACK)
		binary_tree_rb_rebalance_on_delete(child, parent);
#elif defined(BINARY_TREE_RANDOM)
    binary_tree_random_update_weight_on_delete(parent);
#endif

    binary_tree_init_node(old);
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

static cmp_result_t
binary_tree_integer_cmp(struct binary_tree_node *one,
                        struct binary_tree_node *two)
{
    struct bt_node *_one = container_of(one, struct bt_node, tree);
    struct bt_node *_two = container_of(two, struct bt_node, tree);

    return (cmp_result_t)int_sign(_one->num - _two->num);
}

struct binary_tree_node_dot_info {
    FILE *out;
    unsigned long id;
};

static void
__dump_binary_tree_graph(struct binary_tree_node *root,
                         struct binary_tree_node_dot_info *info)
{
    char name[100];
    struct bt_node *node = container_of(root, struct bt_node, tree);

#if defined(BINARY_TREE_AVL)
    sprintf(name, "%u:%d:%d", node->num, BINARY_TREE_AVL_NODE_BALANCE(root), root->height);
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

static void
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

static void
dump_binary_tree(struct binary_tree_node *node, void *data)
{
    struct bt_node *n = container_of(node, struct bt_node, tree);
    printf("%d ", n->num);
}

static int
construct_binary_tree(int *array,
                      int count,
                      struct binary_tree_root *root,
                      struct bt_node **pool)
{
    struct bt_node *nodes;
    int i;

    binary_tree_init_root(root);

    if (!(*pool = malloc(count*sizeof(struct bt_node)))) {
        fprintf(stderr, "Error allocating %d bytes: %s", sizeof(struct bt_node), strerror(errno));
        return -1;
    }

    nodes = *pool;

    for (i=0;i<count;++i) {
        binary_tree_init_node(&nodes[i].tree);

        nodes[i].num = array[i];
        //printf("Inserting %u\n", nodes[i].num);
        binary_tree_add2(root, &nodes[i].tree, binary_tree_integer_cmp);
    }
}

static unsigned int
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

static cmp_result_t
binary_tree_integer_match(struct binary_tree_node *node,
                          void *key)
{
    int _key = *(int *)key;
    struct bt_node *_node = container_of(node, struct bt_node, tree);

    return (cmp_result_t)int_sign(_key - _node->num);
}

static void
binary_tree_int_search(struct binary_tree_node *root,
                   int key,
                   struct sllist *result,
                   int limit)
{
    binary_tree_search(root, (void *)&key, binary_tree_integer_match, result, limit);
}

static void
__check_binary_tree(struct binary_tree_node *node, void *data)
{
    struct bt_node *n = container_of(node, struct bt_node, tree);

#if defined(BINARY_TREE_AVL)
    if (BINARY_TREE_AVL_NODE_BALANCE(node) > 1 || BINARY_TREE_AVL_NODE_BALANCE(node) < -1)
        fprintf(stderr, "AVL BST node(%d) balance violation: %d\n",
                n->num, BINARY_TREE_AVL_NODE_BALANCE(node));
    else if (BINARY_TREE_AVL_NODE_BALANCE(node) == 0
            && ((node->left == BINARY_TREE_EMPTY_BRANCH && node->right != BINARY_TREE_EMPTY_BRANCH)
                    || (node->left != BINARY_TREE_EMPTY_BRANCH && node->right == BINARY_TREE_EMPTY_BRANCH)))
        fprintf(stderr, "AVL BST node(%d) balance violation: node balance is 0 while it has only one child\n", n->num);
    else if (BINARY_TREE_AVL_NODE_BALANCE(node) != 0
            && (node->left == BINARY_TREE_EMPTY_BRANCH && node->right == BINARY_TREE_EMPTY_BRANCH))
        fprintf(stderr, "AVL BST node(%d) balance violation: node balance is %d while no children present\n", n->num, BINARY_TREE_AVL_NODE_BALANCE(node));
#elif defined(BINARY_TREE_RB)
    if ((BINARY_TREE_RB_NODE_COLOR(node) == BINARY_TREE_RB_RED && BINARY_TREE_RB_NODE_COLOR(node->left) == BINARY_TREE_RB_RED)
            || (BINARY_TREE_RB_NODE_COLOR(node) == BINARY_TREE_RB_RED && BINARY_TREE_RB_NODE_COLOR(node->right) == BINARY_TREE_RB_RED))
            fprintf(stderr, "RB BST node(%d) color violation: node %s, left %s, right %s\n",
                    n->num,
                    BINARY_TREE_RB_NODE_COLOR(node)==BINARY_TREE_RB_RED?"red":"black",
                    BINARY_TREE_RB_NODE_COLOR(node->left)==BINARY_TREE_RB_RED?"red":"black",
                    BINARY_TREE_RB_NODE_COLOR(node->right)==BINARY_TREE_RB_RED?"red":"black");
    else if (binary_tree_root_node(node->parent) && BINARY_TREE_RB_NODE_COLOR(node) == BINARY_TREE_RB_RED)
            fprintf(stderr, "RB BST node(%d) color violation: root is red\n", n->num);
    if (node->left == BINARY_TREE_EMPTY_BRANCH
            || node->right == BINARY_TREE_EMPTY_BRANCH) {
        unsigned int bheight = 1;
        while (!binary_tree_root_node(node->parent)) {
            if (node->color == BINARY_TREE_RB_BLACK)
                ++bheight;

            node = node->parent;
        }
        ++bheight;

        if (bheight != *(unsigned int *)data)
            fprintf(stderr, "RB BST node(%d) black height violation: expected %u, calculated %u\n",
                    n->num, *(unsigned int *)data, bheight);
    }
#elif defined(BINARY_TREE_RANDOM)
    if (BINARY_TREE_RANDOM_NODE_WEIGHT(node)
            != BINARY_TREE_RANDOM_NODE_WEIGHT_INCLUSIVE(node->left) + BINARY_TREE_RANDOM_NODE_WEIGHT_INCLUSIVE(node->right))
            fprintf(stderr, "Random BST node(%d) weight violation: node %d, left %d, right %d\n",
                    n->num,
                    BINARY_TREE_RANDOM_NODE_WEIGHT(node),
                    BINARY_TREE_RANDOM_NODE_WEIGHT(node->left),
                    BINARY_TREE_RANDOM_NODE_WEIGHT(node->right));
#elif defined(BINARY_TREE_TREAP)
    if (BINARY_TREE_TREAP_NODE_PRIO(node) < BINARY_TREE_TREAP_NODE_PRIO(node->left)
            || BINARY_TREE_TREAP_NODE_PRIO(node) < BINARY_TREE_TREAP_NODE_PRIO(node->right))
            fprintf(stderr, "Treap node(%d) prio violation: node %d, left %d, right %d\n",
                    n->num,
                    BINARY_TREE_TREAP_NODE_PRIO(node),
                    BINARY_TREE_TREAP_NODE_PRIO(node->left),
                    BINARY_TREE_TREAP_NODE_PRIO(node->right));
#endif
}

static void
check_binary_tree(struct binary_tree_node *root)
{
#if defined(BINARY_TREE_RB)
    unsigned int bheight = 0;
    struct binary_tree_node *n = binary_tree_node(root);
    while (n != BINARY_TREE_EMPTY_BRANCH) {
        if (n->color == BINARY_TREE_RB_BLACK)
            ++bheight;

        n = n->left;
    }
    ++bheight;

    binary_tree_traverse(binary_tree_traverse_type_infix, root, __check_binary_tree, &bheight);
#else
    binary_tree_traverse(binary_tree_traverse_type_infix, root, __check_binary_tree, NULL);
#endif
}

static int
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
                &binary_tree_root.root, dump_binary_tree, NULL);
        printf("\n");
    }

    check_binary_tree(&binary_tree_root.root);

    {
        struct binary_tree_search_result *r;
        struct sllist *e;
        struct bt_node *n;
        unsigned int i;

        usecs = 0;
        for (i=0;i<count;++i) {
            sllist_init(&values);

            gettimeofday(&tb, NULL);
            binary_tree_int_search(&binary_tree_root.root, array[i], &values, 1);
            gettimeofday(&ta, NULL);

            usecs += ta.tv_sec*1000000 + ta.tv_usec - tb.tv_sec*1000000 - tb.tv_usec;

            if (sllist_empty(&values)) {
                fprintf(stderr, "Element %u(%u) was not found\n", i, array[i]);
                goto out;
            }

            e = sllist_first(&values);
            r = container_of(e, struct binary_tree_search_result, list);
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
        binary_tree_int_search(&binary_tree_root.root, key, &values, limit);
        gettimeofday(&ta, NULL);

        if (!sllist_empty(&values)) {
            struct binary_tree_search_result *r;
            struct sllist *e;
            struct bt_node *n;
            int count = 0;

            sllist_for_each(&values, e) {
                ++count;
                r = container_of(e, struct binary_tree_search_result, list);
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
        struct binary_tree_search_result *r;
        struct sllist *e;
        struct bt_node *n;
        unsigned int i;
        unsigned int pivot = count/2;

        for (i=0;i<pivot;++i) {
            sllist_init(&values);

            binary_tree_int_search(&binary_tree_root.root, array[i], &values, 1);

            if (sllist_empty(&values)) {
                 fprintf(stderr, "Element %u(%u) was not found\n", i, array[i]);
                goto out;
            }

            e = sllist_first(&values);
            r = container_of(e, struct binary_tree_search_result, list);
            n = container_of(binary_tree_node(r->node), struct bt_node, tree);
            if (n->num != array[i]) {
                fprintf(stderr, "Found element %u does not match to the search criteria %u\n", n->num, array[i]);
                goto out;
            }

            //printf("Removing %u\n", n->num);
            binary_tree_remove2(&binary_tree_root, binary_tree_node(r->node));

            binary_tree_search_results_free(&values);
        }

        printf("Removed %d tree nodes\n", pivot);

        for (i=pivot;i<count;++i) {
            sllist_init(&values);

            binary_tree_int_search(&binary_tree_root.root, array[i], &values, 1);

            if (sllist_empty(&values)) {
                fprintf(stderr, "Element %u(%u) was not found\n", i, array[i]);
                goto out;
            }

            e = sllist_first(&values);
            r = container_of(e, struct binary_tree_search_result, list);
            n = container_of(binary_tree_node(r->node), struct bt_node, tree);
            if (n->num != array[i]) {
                fprintf(stderr, "Found element %u does not match to the search criteria %u\n", n->num, array[i]);
                goto out;
            }

            binary_tree_search_results_free(&values);
        }

        printf("Rest elements of array were successfully found\n");

        check_binary_tree(&binary_tree_root.root);
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
