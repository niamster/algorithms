#ifndef SSORT_H
#define SSORT_H

#include "sort.h"

/* Implementation of shell sort
 */

enum ssort_gap_sequence {
    SSORT_GAP_SHELL,            /* Original Shell gap sequence  : (N / 2^k) */
    SSORT_GAP_FL,               /* Frank & Lazarus              : (2 * [ N / 2^{k+1} ] + 1) */
    SSORT_GAP_C,                /* Ciura                        : (1, 4, 10, 23, 57, 132, 301, 701) */
};

void shell_sort(void *array,
        unsigned int count,
        unsigned int size,
        assign_t assign,
        swap_t swp,
        compare_t cmp,
        enum ssort_gap_sequence gs);
#endif
