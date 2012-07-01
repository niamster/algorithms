#ifndef MSORT_H
#define MSORT_H

/* Implementation of merge sort
   1st variant allocates memory for each subset and then merges back
   2nd variant is non-recursive and uses only one additional array to store the partially sorted array

   _parallel versions attempt to process subsets simultaneously
 */

void merge_sort1(unsigned int *array,
        unsigned int count);
void merge_sort1_parallel(unsigned int *array,
        unsigned int count,
        unsigned int threads);

void merge_sort2(unsigned int *array,
        unsigned int count);

#endif
