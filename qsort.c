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

struct sort_data;
typedef void (*quick_sort_fn)(struct sort_data *);

struct sort_data {
    /* Sort callbacks */
    quick_sort_fn quick_sort;

    /* Array to sort */
    unsigned int *array;
    unsigned int count;

    unsigned int max_threads;
    pthread_t thread;
};

void *__quick_sort_parallel_wrapper(struct sort_data *data)
{
    data->quick_sort(data);

    return NULL;
}

void __quick_sort_parallel(struct sort_data *data)
{
    pthread_create(&data->thread, NULL, (void *(*)(void*))__quick_sort_parallel_wrapper, data);
}

void __quick_sort_parallel_join(struct sort_data *data)
{
    pthread_join(data->thread, NULL);
}

void
__quick_sort1_v0(unsigned int *array,
                 unsigned int count)
{
    unsigned int m = (count-1)/2;
    unsigned int l, r;
    unsigned int *l_array, *r_array;
    unsigned int l_count = 0, r_count = 0;
    unsigned int pivot;

    if (count <= 1)
        return;

    if (count == 2) {
        if (array[0] > array[1])
            swap(&array[0], &array[1]);

        return;
    }

    l_array = malloc(count*sizeof(unsigned int));
    r_array = malloc(count*sizeof(unsigned int));

    pivot = array[m];

    for (l=0;l<count;++l) {
        if (l == m)
            continue;

        if (array[l] <= pivot)
            l_array[l_count++] = array[l];
        else
            r_array[r_count++] = array[l];
    }

    __quick_sort1_v0(l_array, l_count);
    __quick_sort1_v0(r_array, r_count);

    for (l=0;l<l_count;++l)
        array[l] = l_array[l];
    array[l++] = pivot;

    for (r=0;r<r_count;++l, ++r)
        array[l] = r_array[r];

    free(l_array);
    free(r_array);
}

void
__quick_sort1_v1(struct sort_data *data)
{
    struct sort_data sort_data[] = {
        [0] = {
            .quick_sort = __quick_sort1_v1,
        },
        [1] = {
            .quick_sort = __quick_sort1_v1,
        },
    };
    unsigned int *array = data->array;
    unsigned int count = data->count;
    unsigned int m = (count-1)/2;
    unsigned int l, r;
    unsigned int *l_array, *r_array;
    unsigned int l_count = 0, r_count = 0;
    unsigned int pivot;

    if (count <= 1)
        return;

    if (count == 2) {
        if (array[0] > array[1])
            swap(&array[0], &array[1]);

        return;
    }

    l_array = malloc(count*sizeof(unsigned int));
    r_array = malloc(count*sizeof(unsigned int));

    pivot = array[m];

    for (l=0;l<count;++l) {
        if (l == m)
            continue;

        if (array[l] <= pivot)
            l_array[l_count++] = array[l];
        else
            r_array[r_count++] = array[l];
    }

    sort_data[0].array = l_array;
    sort_data[0].count = l_count;
    sort_data[1].array = r_array;
    sort_data[1].count = r_count;
    if (data->max_threads > 1) {
        data->max_threads -= 2;
        sort_data[0].max_threads = data->max_threads;
        sort_data[1].max_threads = data->max_threads;
        __quick_sort_parallel(&sort_data[0]);
        __quick_sort_parallel(&sort_data[1]);

        __quick_sort_parallel_join(&sort_data[0]);
        __quick_sort_parallel_join(&sort_data[1]);
    } else {
        __quick_sort1_v1(&sort_data[0]);
        __quick_sort1_v1(&sort_data[1]);
    }

    for (l=0;l<l_count;++l)
        array[l] = l_array[l];
    array[l++] = pivot;

    for (r=0;r<r_count;++l, ++r)
        array[l] = r_array[r];

    free(l_array);
    free(r_array);
}

void
__quick_sort2_v0(unsigned int *array,
                 unsigned int l, unsigned int r)
{
    long count = (long)r-(long)l+1;
    unsigned int m = (l+r)/2;
    unsigned int i, s;
    unsigned int pivot;

    if (count <= 1)
        return;

    if (count == 2) {
        if (array[l] > array[r])
            swap(&array[l], &array[r]);

        return;
    }

    pivot = array[m];

    swap(&array[r], &array[m]);

    for (s=i=l;i<r;++i)
        if (array[i] <= pivot) {
            if (s != i)
                swap(&array[s], &array[i]);
            ++s;
        }

    swap(&array[s], &array[r]);

    if (s != l)
        __quick_sort2_v0(array, l, s-1);
    __quick_sort2_v0(array, s, r);
}

void
__quick_sort2_v1(struct sort_data *data)
{
    struct sort_data sort_data[] = {
        [0] = {
            .quick_sort = __quick_sort2_v1,
        },
        [1] = {
            .quick_sort = __quick_sort2_v1,
        },
    };
    unsigned int *array = data->array;
    unsigned int count = data->count;
    unsigned int l = 0, r = count-1;
    unsigned int m = (count-1)/2;
    unsigned int i, s;
    unsigned int pivot;

    if (count <= 1)
        return;

    if (count == 2) {
        if (array[0] > array[1])
            swap(&array[0], &array[1]);

        return;
    }

    pivot = array[m];

    swap(&array[r], &array[m]);

    for (s=i=l;i<r;++i)
        if (array[i] <= pivot) {
            if (s != i)
                swap(&array[s], &array[i]);
            ++s;
        }

    swap(&array[s], &array[r]);

    sort_data[0].array = array;
    sort_data[0].count = s;
    sort_data[1].array = array+s+1;
    sort_data[1].count = r-s;

    if (data->max_threads > 1) {
        data->max_threads -= 2;
        sort_data[0].max_threads = data->max_threads;
        sort_data[1].max_threads = data->max_threads;
        __quick_sort_parallel(&sort_data[0]);
        __quick_sort_parallel(&sort_data[1]);

        __quick_sort_parallel_join(&sort_data[0]);
        __quick_sort_parallel_join(&sort_data[1]);
    } else {
        __quick_sort2_v1(&sort_data[0]);
        __quick_sort2_v1(&sort_data[1]);
    }
}

void
quick_sort1(unsigned int *array,
            unsigned int count)
{
    __quick_sort1_v0(array, count);
}

void
quick_sort2(unsigned int *array,
            unsigned int count)
{
    __quick_sort2_v0(array, 0, count-1);
}

void quick_sort1_parallel(unsigned int *array,
                          unsigned int count,
                          unsigned int threads)
{
    struct sort_data sort_data = {
        .array = array,
        .count = count,
        .max_threads = threads,
    };
    __quick_sort1_v1(&sort_data);
}

void quick_sort2_parallel(unsigned int *array,
                          unsigned int count,
                          unsigned int threads)
{
    struct sort_data sort_data = {
        .array = array,
        .count = count,
        .max_threads = threads,
    };
    __quick_sort2_v1(&sort_data);
}

#ifdef QSORT_MAIN
enum sort_variant {
    sort_variant_qs1,
    sort_variant_qs2
};

int
usage(const char *prog)
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "%s --sort-variant|-s QS1|QS2 [--threads|-t <num>] [--input-data|-i <path>] [--count|-c <num>] [--output-data|-o <path>] [--dump]\n", prog);
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
    unsigned int *array;

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

    printf("Elements: %d\n", count);
    if (dump) {
        printf("\nOriginal array:\n");
        print_array(array, count);
    }

    switch (sort_variant) {
        case sort_variant_qs1:
            printf("QS1(%d threads)\n", threads);
            gettimeofday(&tb, NULL);
            if (threads)
                quick_sort1_parallel(array, count, threads);
            else
                quick_sort1(array, count);
            gettimeofday(&ta, NULL);
            break;

        case sort_variant_qs2:
            printf("QS2(%d threads)\n", threads);
            gettimeofday(&tb, NULL);
            if (threads)
                quick_sort2_parallel(array, count, threads);
            else
                quick_sort2(array, count);
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

    return 0;
}
#endif
