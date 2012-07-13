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
#include "qsort.h"
#include "hsort.h"
#include "msort.h"

enum sort_variant {
    sort_variant_qs1,
    sort_variant_qs2,
    sort_variant_hs,
    sort_variant_ms1,
    sort_variant_ms2,
};

enum torture_step {
    sort_unsorted,
    sort_pre_sorted,
    sort_backward_sorted,

    torture_steps_qty,
};

static void
uint_assign(void *a, void *b)
{
    *(unsigned int *)a = *(unsigned int *)b;
}

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
    fprintf(stderr, "%s --sort-variant|-s QS1|QS2|HS|MS1|MS2 [--threads|-t <num>] [--input-data|-i <path>] [--count|-c <num>] [--output-data|-o <path>] [--dump]\n", prog);
    return 1;
}

int main(unsigned int argc, char **argv)
{
    struct timeval tb, ta;
    unsigned long secs, msecs, usecs;

    int dump = 0;
    int sort_variant = -1;
    unsigned int count = 0;
    unsigned int threads = 0;
    const char *input_data = "/dev/random", *output_data = NULL;
    unsigned int *array, *origin = NULL;
    enum torture_step ts;

    int opt;
    const struct option options[] = {
        {"sort-variant",  required_argument, NULL, 's'},
        {"threads",       required_argument, NULL, 't'},
        {"input-data",    required_argument, NULL, 'i'},
        {"output-data",   required_argument, NULL, 'o'},
        {"count",         required_argument, NULL, 'c'},
        {"dump",          no_argument,       &dump, 1},
        {0,               0,                 0,    0}
    };

    while ((opt = getopt_long(argc, argv, "s:t:i:o:c:", options, NULL)) != -1) {
        switch (opt) {
            case 's':
                if (!strncmp(optarg, "QS1", 3))
                    sort_variant = sort_variant_qs1;
                else if (!strncmp(optarg, "QS2", 3))
                    sort_variant = sort_variant_qs2;
                else if (!strncmp(optarg, "HS", 2))
                    sort_variant = sort_variant_hs;
                else if (!strncmp(optarg, "MS1", 3))
                    sort_variant = sort_variant_ms1;
                else if (!strncmp(optarg, "MS2", 3))
                    sort_variant = sort_variant_ms2;
                else
                    return usage(argv[0]);
                break;
            case 't':
                threads = atoi(optarg);
                break;
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

    if (sort_variant == -1)
        return usage(argv[0]);

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
        printf("Original array:\n");
        print_array(array, count);
    }

    switch (sort_variant) {
        case sort_variant_qs1:
            printf("Quick sort v1(%d threads)\n", threads);
            break;

        case sort_variant_qs2:
            printf("Quick sort v2(%d threads)\n", threads);
            break;

        case sort_variant_hs:
            printf("Heap sort\n", threads);
            break;

        case sort_variant_ms1:
            printf("Merge sort v1(%d threads)\n", threads);
            break;

        case sort_variant_ms2:
            printf("Merge sort v2(%d threads)\n", threads);
            break;
    }

    for (ts=sort_unsorted; ts<torture_steps_qty; ++ts) {
        switch (ts) {
            case sort_unsorted:
                printf("Processing unsorted array\n");
                break;

            case sort_pre_sorted:
                printf("Processing sorted array\n");
                break;

            case sort_backward_sorted:
                printf("Processing backward-sorted array\n");
                break;
        }

        switch (sort_variant) {
            case sort_variant_qs1:
                gettimeofday(&tb, NULL);
                if (threads)
                    quick_sort1_parallel(array, count, threads, sizeof(unsigned int), uint_assign, uint_swap, uint_cmp);
                else
                    quick_sort1(array, count, sizeof(unsigned int), uint_assign, uint_swap, uint_cmp);
                gettimeofday(&ta, NULL);
                break;

            case sort_variant_qs2:
                gettimeofday(&tb, NULL);
                if (threads)
                    quick_sort2_parallel(array, count, threads, sizeof(unsigned int), uint_swap, uint_cmp);
                else
                    quick_sort2(array, count, sizeof(unsigned int), uint_swap, uint_cmp);
                gettimeofday(&ta, NULL);
                break;

            case sort_variant_hs:
                gettimeofday(&tb, NULL);
                heap_sort(array, count, sizeof(unsigned int), uint_swap, uint_cmp);
                gettimeofday(&ta, NULL);
                break;

            case sort_variant_ms1:
                gettimeofday(&tb, NULL);
                if (threads)
                    merge_sort1_parallel(array, count, threads, sizeof(unsigned int), uint_assign, uint_swap, uint_cmp);
                else
                    merge_sort1(array, count, sizeof(unsigned int), uint_assign, uint_swap, uint_cmp);
                gettimeofday(&ta, NULL);
                break;

            case sort_variant_ms2:
                gettimeofday(&tb, NULL);
                merge_sort2(array, count, sizeof(unsigned int), uint_assign, uint_swap, uint_cmp);
                gettimeofday(&ta, NULL);
                break;
        }

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

        switch (ts) {
            case sort_unsorted:
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
                break;

            case sort_pre_sorted:
                if (1) {
                    unsigned int i, j;
                    memcpy(origin, array, count*sizeof(unsigned int));
                    for (i=count-1,j=0;j<count;--i,++j)
                        array[j] = origin[i];
                }
                break;

            case sort_backward_sorted:
                break;
        }
    }

  out:
    free(array);
    if (origin)
        free(origin);

    return 0;
}
