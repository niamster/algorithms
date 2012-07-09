#ifndef MSORT_H
#define MSORT_H

#include "sort.h"

/* Implementation of merge sort
   1st variant allocates memory for each subset and then merges back
   2nd variant is non-recursive and uses only one additional array to store the partially sorted array

   _parallel versions attempt to process subsets simultaneously
 */

void merge_sort1(void *array,
        unsigned int count,
        unsigned int size,
        assign_t assign,
        swap_t swp,
        compare_t cmp);
void merge_sort1_parallel(void *array,
        unsigned int count,
        unsigned int threads,
        unsigned int size,
        assign_t assign,
        swap_t swp,
        compare_t cmp);

void merge_sort2(void *array,
        unsigned int count,
        unsigned int size,
        assign_t assign,
        swap_t swp,
        compare_t cmp);

#endif
