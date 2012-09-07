#ifndef ISORT_H
#define ISORT_H

#include "sort.h"

/* Implementation of insertion sort
 */

void insertion_sort(void *array,
        unsigned int count,
        unsigned int size,
        assign_t assign,
        swap_t swp,
        compare_t cmp);
#endif
