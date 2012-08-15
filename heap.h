#ifndef HEAP_H
#define HEAP_H

#include <stdbool.h>
#include <stdlib.h>

#include "hsort.h"

struct heap {
    unsigned int size;
    unsigned int max;
    assign_t assign;
    swap_t swp;
    compare_t cmp;
    unsigned int nsize;
    void *nodes;
};

static inline bool heap_init(struct heap *heap, unsigned int max,
        unsigned int size,
        assign_t assign,
        swap_t swp,
        compare_t cmp)
{
    heap->nodes = malloc(size*max);
    if (!heap->nodes)
        return false;

    heap->size = 0;
    heap->max = max;
    heap->swp = swp;
    heap->assign = assign;
    heap->cmp = cmp;
    heap->nsize = size;

    return true;
}

static inline void heap_destroy(struct heap *heap)
{
    free(heap->nodes);
}

static inline bool
heap_push(struct heap *heap, void *data)
{
    char *nodes = heap->nodes;
    typeof(heap->size) size = heap->size;
    typeof(heap->nsize) nsize = heap->nsize;

    if (size == heap->max)
        return false;

    heap->assign(&nodes[size*nsize], data);
    ++size;

    heap_sort(nodes, size, heap->nsize, heap->swp, heap->cmp);

    heap->size = size;

    return true;
}

static inline void *
heap_get_top(struct heap *heap)
{
    if (0 == heap->size)
        return NULL;

    return heap->nodes;
}

static inline bool
heap_pop_top(struct heap *heap, void *data)
{
    char *nodes = heap->nodes;
    typeof(heap->size) size = heap->size;
    typeof(heap->nsize) nsize = heap->nsize;

    if (0 == size)
        return false;

    if (data)
        heap->assign(data, nodes);

    --size;

    if (size) {
        heap->assign(nodes, &nodes[size*nsize]);
        heap_sort(nodes, size, heap->nsize, heap->swp, heap->cmp);
    }

    heap->size = size;

    return true;
}

static inline void *
heap_get_bottom(struct heap *heap)
{
    char *nodes = heap->nodes;
    typeof(heap->size) size = heap->size;
    typeof(heap->nsize) nsize = heap->nsize;

    if (0 == size)
        return NULL;

    return &nodes[(size-1)*nsize];
}

static inline bool
heap_pop_bottom(struct heap *heap, void *data)
{
    char *nodes = heap->nodes;
    typeof(heap->size) size = heap->size;
    typeof(heap->nsize) nsize = heap->nsize;

    if (0 == size)
        return false;

    --size;

    if (data)
        heap->assign(data, &nodes[size*nsize]);

    heap->size = size;

    return true;
}

#endif
