#ifndef HSORT_H
#define HSORT_H

#include "sort.h"

void heap_sort(void *array,
        unsigned int count,
        unsigned int size,
        swap_t swp,
        compare_t cmp);
#endif
