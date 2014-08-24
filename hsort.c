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
#include "heap.h"
#include "hsort.h"

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

    __heap_build(array, count, size, swp, cmp);

    while (--count > 0) {
        swp(array, array+count*size);
        __heap_push_down(array, 0, count, size, swp, cmp);
    }
}
