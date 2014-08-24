#ifndef TSORT_H
#define TSORT_H

#include "sort.h"

/* Implementation of Timsort
 */

void tim_sort(void *array,
        unsigned int count,
        unsigned int size,
        assign_t assign,
        swap_t swp,
        compare_t cmp);
#endif
