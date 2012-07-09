#ifndef QSORT_H
#define QSORT_H

#include "sort.h"

/* Implementation of qsort
   1st variant uses merge-like algorithm to divide an array into the subsets: allocates memory for each subset and then merges back
   2nd variant uses the same array to process the subset

   _parallel versions attempt to process subsets simultaneously
 */

void quick_sort1(void *array,
        unsigned int count,
        unsigned int size,
        assign_t assign,
        swap_t swp,
        compare_t cmp);
void quick_sort1_parallel(void *array,
        unsigned int count,
        unsigned int threads,
        unsigned int size,
        assign_t assign,
        swap_t swp,
        compare_t cmp);

void quick_sort2(void *array,
        unsigned int count,
        unsigned int size,
        swap_t swp,
        compare_t cmp);
void quick_sort2_parallel(void *array,
        unsigned int count,
        unsigned int threads,
        unsigned int size,
        swap_t swp,
        compare_t cmp);

#endif
