#ifndef QSORT_H
#define QSORT_H

/* Implementation of qsort
   1st variant uses merge-like algorithm to divide an array into the subsets: allocates memory for for each and then merges back
   2nd variant uses the same array to process the subset

   _parallel versions attempt to process subsets simultaneously
 */

void quick_sort1(unsigned int *array,
        unsigned int count);
void quick_sort1_parallel(unsigned int *array,
        unsigned int count,
        unsigned int threads);

void quick_sort2(unsigned int *array,
        unsigned int count);
void quick_sort2_parallel(unsigned int *array,
        unsigned int count,
        unsigned int threads);

#endif
