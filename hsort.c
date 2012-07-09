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

#ifdef HSORT_MAIN
static void
uint_swap(void *a, void *b)
{
    swap((unsigned int *)a, (unsigned int *)b);
}

static cmp_result_t
uint_cmp(void *a, void *b)
{
    return (cmp_result_t)int_sign(*(unsigned int *)a - *(unsigned int *)b);
}

static int
usage(const char *prog)
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "%s [--input-data|-i <path>] [--count|-c <num>] [--output-data|-o <path>] [--dump]\n", prog);
    return 1;
}

int main(unsigned int argc, char **argv)
{
    struct timeval tb, ta;
    unsigned long secs, msecs, usecs;

    int dump = 0;
    unsigned int count = 0;
    const char *input_data = "/dev/random", *output_data = NULL;
    unsigned int *array, *origin = NULL;

    int opt;
    const struct option options[] = {
        {"input-data",    required_argument, NULL, 'i'},
        {"output-data",   required_argument, NULL, 'o'},
        {"count",         required_argument, NULL, 'c'},
        {"dump",          no_argument,       &dump, 1},
        {0,               0,                 0,    0}
    };

    while ((opt = getopt_long(argc, argv, "i:o:c:", options, NULL)) != -1) {
        switch (opt) {
            case 'c':
                count = atoi(optarg);
                break;
            case 'i':
                input_data = optarg;
                break;
            case 'o':
                output_data = optarg;
                break;
            case 0:
                break;
            default:
                return usage(argv[0]);
        }
    }

    if (generate_array(&array, &count, input_data) == -1)
        return 1;

    origin = malloc(count*sizeof(unsigned int));
    if (!origin) {
        fprintf(stderr, "Unable to allocate %u bytes to store origin array\n", count*sizeof(unsigned int));
        goto out;
    }
    memcpy(origin, array, count*sizeof(unsigned int));

    printf("Elements: %d\n", count);
    if (dump) {
        printf("\nOriginal array:\n");
        print_array(array, count);
    }

    gettimeofday(&tb, NULL);
    heap_sort(array, count, sizeof(unsigned int), uint_swap, uint_cmp);
    gettimeofday(&ta, NULL);

    usecs = ta.tv_sec*1000000 + ta.tv_usec - tb.tv_sec*1000000 - tb.tv_usec;
    secs = usecs/1000000;
    usecs %= 1000000;
    msecs = usecs/1000;
    usecs %= 1000;
    printf("Sort time: %lu seconds %lu msecs %lu usecs\n", secs, msecs, usecs);

    {
        unsigned int i;
        for (i=1;i<count;++i)
            if (array[i-1] > array[i])
                fprintf(stderr, "Array is not sorted properly %d@%d > %d@%d\n",
                        array[i-1], i-1, array[i], i);
    }

    {
        unsigned int i, j;
        for (i=0;i<count;++i) {
            unsigned int o = origin[i];
            for (j=0;j<count;++j)
                if (array[j] == o)
                    break;

            if (j==count)
                fprintf(stderr, "Sorted array does not contain %u from origin\n", o);
        }
    }

    if (dump) {
        printf("\nSorted array:\n");
        print_array(array, count);
    }

    if (output_data) {
        int d = open(output_data, O_WRONLY|O_CREAT, 0666);
        if (d == -1) {
            fprintf(stderr, "Error opening %s: %s", output_data, strerror(errno));
            goto out;
        }
        ftruncate(d, 0);
        if (write(d, array, count*sizeof(unsigned int)) == -1)
            fprintf(stderr, "Error error writing to %s: %s", output_data, strerror(errno));
        close(d);
    }

  out:
    free(array);
    if (origin)
        free(origin);

    return 0;
}
#endif
