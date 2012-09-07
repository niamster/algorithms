#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/time.h>
#include <pthread.h>
#include <errno.h>
#include <getopt.h>

#include "helpers.h"
#include "isort.h"

void insertion_sort(void *array,
        unsigned int count,
        unsigned int size,
        assign_t assign,
        swap_t swp,
        compare_t cmp)
{
    int i, j;
    unsigned int t[size/sizeof(unsigned int)+1];
    
    if (1 == count)
        return;

    if (2 == count) {
        if (cmp(array, array+size) == cmp_result_greater)
            swp(array, array+size);

        return;
    }

    for (i=1;i<count;++i) {
        assign(t, array+size*i);

        for (j=i-1;j>=0 && cmp(array+size*j, t) == cmp_result_greater;--j)
            assign(array+size*(j+1), array+size*j);

        if (i != j+1)
            assign(array+size*(j+1), t);
    }
}
