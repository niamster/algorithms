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

struct sort_data;
typedef void (*quick_sort_fn)(struct sort_data *);

struct sort_data {
    /* Sort callbacks */
    quick_sort_fn quick_sort;

    /* Array to sort */
    void *array;
    unsigned int count;

    unsigned int max_threads;
    pthread_t thread;

    unsigned int size;
    assign_t assign;
    swap_t swp;
    compare_t cmp;
};

static void *
__quick_sort_parallel_wrapper(struct sort_data *data)
{
    data->quick_sort(data);

    return NULL;
}

static void
__quick_sort_parallel(struct sort_data *data)
{
    pthread_create(&data->thread, NULL, (void *(*)(void*))__quick_sort_parallel_wrapper, data);
}

static void
__quick_sort_parallel_join(struct sort_data *data)
{
    pthread_join(data->thread, NULL);
}

static void
__quick_sort1_v0(void *array,
        unsigned int count,
        unsigned int size,
        assign_t assign,
        swap_t swp,
        compare_t cmp)
{
    unsigned int m = (count-1)/2;
    unsigned int l, r;
    void *l_array, *r_array;
    unsigned int l_count = 0, r_count = 0;
    void *pivot;

    if (count <= 1)
        return;

    if (count == 2) {
        if (cmp(array, array+size) == cmp_result_greater)
            swp(array, array+size);

        return;
    }

    l_array = malloc(count*size);
    r_array = malloc(count*size);

    pivot = malloc(size);
    assign(pivot, array+m*size);

    for (l=0;l<count;++l) {
        if (l == m)
            continue;

        if (cmp(array+l*size, pivot) == cmp_result_less)
            assign(l_array+l_count*size, array+l*size), ++l_count;
        else
            assign(r_array+r_count*size, array+l*size), ++r_count;
    }

    __quick_sort1_v0(l_array, l_count, size, assign, swp, cmp);
    __quick_sort1_v0(r_array, r_count, size, assign, swp, cmp);

    memcpy(array, l_array, l_count*size);
    assign(array+l_count*size, pivot);
    memcpy(array+(l_count+1)*size, r_array, r_count*size);

    free(l_array);
    free(r_array);
    free(pivot);
}

static void
__quick_sort1_v1(struct sort_data *data)
{
    struct sort_data sort_data[] = {
        [0] = {
            .quick_sort = __quick_sort1_v1,
            .size = data->size,
            .assign = data->assign,
            .swp = data->swp,
            .cmp = data->cmp,
        },
        [1] = {
            .quick_sort = __quick_sort1_v1,
            .size = data->size,
            .assign = data->assign,
            .swp = data->swp,
            .cmp = data->cmp,
        },
    };
    void *array = data->array;
    unsigned int count = data->count;
    unsigned int m = (count-1)/2;
    unsigned int l, r;
    void *l_array, *r_array;
    unsigned int l_count = 0, r_count = 0;
    void *pivot;
    unsigned int size = data->size;
    assign_t assign = data->assign;
    swap_t swp = data->swp;
    compare_t cmp = data->cmp;

    if (count <= 1)
        return;

    if (count == 2) {
        if (cmp(array, array+size) == cmp_result_greater)
            swp(array, array+size);

        return;
    }

    l_array = malloc(count*size);
    r_array = malloc(count*size);

    pivot = malloc(size);
    assign(pivot, array+m*size);

    for (l=0;l<count;++l) {
        if (l == m)
            continue;

        if (cmp(array+l*size, pivot) == cmp_result_less)
            assign(l_array+l_count*size, array+l*size), ++l_count;
        else
            assign(r_array+r_count*size, array+l*size), ++r_count;
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

    memcpy(array, l_array, l_count*size);
    assign(array+l_count*size, pivot);
    memcpy(array+(l_count+1)*size, r_array, r_count*size);

    free(l_array);
    free(r_array);
    free(pivot);
}

static void
__quick_sort2_v0(void *array,
        unsigned int l, unsigned int r,
        unsigned int size,
        swap_t swp,
        compare_t cmp)
{
    long count = (long)r-(long)l+1;
    unsigned int m = (l+r)/2;
    unsigned int i, s;
    cmp_result_t res;

    if (count <= 1)
        return;

    if (count == 2) {
        if (cmp(array+l*size, array+r*size) == cmp_result_greater)
            swp(array+l*size, array+r*size);

        return;
    }

    swp(array+r*size, array+m*size);

    for (s=i=l;i<r;++i) {
        res = cmp(array+i*size, array+r*size);
        if (cmp_result_less == res || cmp_result_equal == res) {
            if (s != i)
                swp(array+s*size, array+i*size);
            ++s;
        }
    }

    swp(array+s*size, array+r*size);

    if (s != l)
        __quick_sort2_v0(array, l, s-1, size, swp, cmp);
    __quick_sort2_v0(array, s, r, size, swp, cmp);
}

static void
__quick_sort2_v1(struct sort_data *data)
{
    struct sort_data sort_data[] = {
        [0] = {
            .quick_sort = __quick_sort2_v1,
            .size = data->size,
            .swp = data->swp,
            .cmp = data->cmp,
        },
        [1] = {
            .quick_sort = __quick_sort2_v1,
            .size = data->size,
            .swp = data->swp,
            .cmp = data->cmp,
        },
    };
    void *array = data->array;
    unsigned int count = data->count;
    unsigned int l = 0, r = count-1;
    unsigned int m = (count-1)/2;
    unsigned int i, s;
    cmp_result_t res;
    unsigned int size = data->size;
    swap_t swp = data->swp;
    compare_t cmp = data->cmp;

    if (count <= 1)
        return;

    if (count == 2) {
        if (cmp(array, array+size) == cmp_result_greater)
            swp(array, array+size);

        return;
    }

    swp(array+r*size, array+m*size);

    for (s=i=l;i<r;++i) {
        res = cmp(array+i*size, array+r*size);
        if (cmp_result_less == res || cmp_result_equal == res) {
            if (s != i)
                swp(array+s*size, array+i*size);
            ++s;
        }
    }

    swp(array+s*size, array+r*size);

    sort_data[0].array = array;
    sort_data[0].count = s;
    sort_data[1].array = array+(s+1)*size;
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
quick_sort1(void *array,
        unsigned int count,
        unsigned int size,
        assign_t assign,
        swap_t swp,
        compare_t cmp)
{
    __quick_sort1_v0(array, count, size, assign, swp, cmp);
}

void
quick_sort2(void *array,
        unsigned int count,
        unsigned int size,
        swap_t swp,
        compare_t cmp)
{
    if (count <= 1)
        return;

    __quick_sort2_v0(array, 0, count-1, size, swp, cmp);
}

void quick_sort1_parallel(void *array,
        unsigned int count,
        unsigned int threads,
        unsigned int size,
        assign_t assign,
        swap_t swp,
        compare_t cmp)
{
    struct sort_data sort_data = {
        .array = array,
        .count = count,
        .max_threads = threads,
        .size = size,
        .assign = assign,
        .swp = swp,
        .cmp = cmp,
    };
    __quick_sort1_v1(&sort_data);
}

void quick_sort2_parallel(void *array,
        unsigned int count,
        unsigned int threads,
        unsigned int size,
        swap_t swp,
        compare_t cmp)
{
    struct sort_data sort_data = {
        .array = array,
        .count = count,
        .max_threads = threads,
        .size = size,
        .swp = swp,
        .cmp = cmp,
    };
    __quick_sort2_v1(&sort_data);
}

#ifdef QSORT_MAIN
enum sort_variant {
    sort_variant_qs1,
    sort_variant_qs2
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
    unsigned int *array, *origin = NULL;

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

    switch (sort_variant) {
        case sort_variant_qs1:
            printf("QS1(%d threads)\n", threads);
            gettimeofday(&tb, NULL);
            if (threads)
                quick_sort1_parallel(array, count, threads, sizeof(unsigned int), uint_assign, uint_swap, uint_cmp);
            else
                quick_sort1(array, count, sizeof(unsigned int), uint_assign, uint_swap, uint_cmp);
            gettimeofday(&ta, NULL);
            break;

        case sort_variant_qs2:
            printf("QS2(%d threads)\n", threads);
            gettimeofday(&tb, NULL);
            if (threads)
                quick_sort2_parallel(array, count, threads, sizeof(unsigned int), uint_swap, uint_cmp);
            else
                quick_sort2(array, count, sizeof(unsigned int), uint_swap, uint_cmp);
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
