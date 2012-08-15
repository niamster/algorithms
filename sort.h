#ifndef SORT_H
#define SORT_H

#include "helpers.h"

typedef void (*swap_t)(void *, void *);
typedef void (*assign_t)(void *, void *);
typedef cmp_result_t (*compare_t)(void *, void *);

#endif
