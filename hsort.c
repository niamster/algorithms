#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/time.h>
#include <errno.h>
#include <getopt.h>
#include <stdbool.h>

#include "helpers.h"
#include "hsort.h"

static void
heap_push(void *array,
        unsigned int e,
        unsigned int count,
        unsigned int size,
        swap_t swp,
        compare_t cmp)
{
    unsigned int l, r;
    unsigned int m;

    for (;;) {
        l = 2*e + 1;
        r = 2*e + 2;

        m = e;

        if (l < count && cmp(array+l*size, array+m*size) == cmp_result_greater)
            m = l;

        if (r < count && cmp(array+r*size, array+m*size) == cmp_result_greater)
            m = r;

        if (m == e)
            break;

        swp(array+m*size, array+e*size);
        e = m;
    }
}

static void
heap_build(void *array,
        unsigned int count,
        unsigned int size,
        swap_t swp,
        compare_t cmp)
{
    long m = (count-1)/2;

    for (;m>=0;--m)
        heap_push(array, m, count, size, swp, cmp);
}

void
heap_sort(void *array,
        unsigned int count,
        unsigned int size,
        swap_t swp,
        compare_t cmp)
{
    if (count <= 1)
        return;

    if (count == 2) {
        if (cmp(array, array+size) == cmp_result_greater)
            swp(array, array+size);

        return;
    }

    heap_build(array, count, size, swp, cmp);

    while (--count > 0) {
        swp(array, array+count*size);
        heap_push(array, 0, count, size, swp, cmp);
    }
}
