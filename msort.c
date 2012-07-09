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
#include "msort.h"

struct sort_data;
typedef void (*merge_sort_fn)(struct sort_data *);

struct sort_data {
    /* Sort callbacks */
    merge_sort_fn merge_sort;

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
__merge_sort_parallel_wrapper(struct sort_data *data)
{
    data->merge_sort(data);

    return NULL;
}

static void
__merge_sort_parallel(struct sort_data *data)
{
    pthread_create(&data->thread, NULL, (void *(*)(void*))__merge_sort_parallel_wrapper, data);
}

static void
__merge_sort_parallel_join(struct sort_data *data)
{
    pthread_join(data->thread, NULL);
}

static void
__merge_sort1_v0(void *array,
        unsigned int count,
        unsigned int size,
        assign_t assign,
        swap_t swp,
        compare_t cmp)
{
    unsigned int m = (count-1)/2;
    unsigned int i, l, r;
    void *l_array, *r_array;

    if (count <= 1)
        return;

    if (count == 2) {
        if (cmp(array, array+size) == cmp_result_greater)
            swp(array, array+size);

        return;
    }

    l_array = malloc(m*size);
    r_array = malloc((count-m)*size);

    memcpy(l_array, array, m*size);
    memcpy(r_array, array+m*size, (count-m)*size);

    __merge_sort1_v0(l_array, m, size, assign, swp, cmp);
    __merge_sort1_v0(r_array, count-m, size, assign, swp, cmp);

    for (i=0,l=0,r=0;i<count;++i) {
        if (r >= (count - m) || (l < m && cmp(l_array+l*size, r_array+r*size) == cmp_result_less))
            assign(array+i*size, l_array+l*size), ++l;
        else
            assign(array+i*size, r_array+r*size), ++r;
    }

    free(l_array);
    free(r_array);
}

static void
__merge_sort1_v1(struct sort_data *data)
{
    struct sort_data sort_data[] = {
        [0] = {
            .merge_sort = __merge_sort1_v1,
            .size = data->size,
            .assign = data->assign,
            .swp = data->swp,
            .cmp = data->cmp,
        },
        [1] = {
            .merge_sort = __merge_sort1_v1,
            .size = data->size,
            .assign = data->assign,
            .swp = data->swp,
            .cmp = data->cmp,
        },
    };
    void *array = data->array;
    unsigned int count = data->count;
    unsigned int m = (count-1)/2;
    unsigned int i, l, r;
    void *l_array, *r_array;
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

    l_array = malloc(m*size);
    r_array = malloc((count-m)*size);

    memcpy(l_array, array, m*size);
    memcpy(r_array, array+m*size, (count-m)*size);

    sort_data[0].array = l_array;
    sort_data[0].count = m;
    sort_data[1].array = r_array;
    sort_data[1].count = count-m;
    if (data->max_threads > 1) {
        data->max_threads -= 2;
        sort_data[0].max_threads = data->max_threads;
        sort_data[1].max_threads = data->max_threads;
        __merge_sort_parallel(&sort_data[0]);
        __merge_sort_parallel(&sort_data[1]);

        __merge_sort_parallel_join(&sort_data[0]);
        __merge_sort_parallel_join(&sort_data[1]);
    } else {
        __merge_sort1_v1(&sort_data[0]);
        __merge_sort1_v1(&sort_data[1]);
    }

    for (i=0,l=0,r=0;i<count;++i) {
        if (r >= (count - m) || (l < m && cmp(l_array+l*size, r_array+r*size) == cmp_result_less))
            assign(array+i*size, l_array+l*size), ++l;
        else
            assign(array+i*size, r_array+r*size), ++r;
    }

    free(l_array);
    free(r_array);
}

static void
__merge_sort2_v0(void *array,
        unsigned int count,
        unsigned int size,
        assign_t assign,
        swap_t swp,
        compare_t cmp)
{
    unsigned int chunks = count/2;
    unsigned int tail = count%2;
    void *origin = array;
    void *out;
    unsigned int i, j, k, t, c;
    unsigned int l, r;
    void *l_array, *r_array;

    if (count <= 1)
        return;

    if (count == 2) {
        if (cmp(array, array+size) == cmp_result_greater)
            swp(array, array+size);

        return;
    }

    for (i=0;i<chunks;++i) {
        t = i*2;
        if (cmp(array+t*size, array+(t+1)*size) == cmp_result_greater)
            swp(array+t*size, array+(t+1)*size);
    }

    out = malloc(count*size);
    if (!out)
        return;

    count -= tail;
    c = 2;
  merge:
    k = c*2;

    for (i=0;i<count/k;++i) {
        t = i*k;
        l_array = array+t*size;
        r_array = array+(t+c)*size;

        for (j=t,r=l=0;j<t+k;++j) {
            if (r >= c || (l < c && cmp(l_array+l*size, r_array+r*size) == cmp_result_less))
                assign(out+j*size, l_array+l*size), ++l;
            else
                assign(out+j*size, r_array+r*size), ++r;
        }
    }

    if (count%k) {
        t = i*k;
        l_array = array+t*size;
        r_array = array+(t+c)*size;

        for (j=t,r=l=0;j<count+tail;++j) {
            if (r >= tail || (l < c && cmp(l_array+l*size, r_array+r*size) == cmp_result_less))
                assign(out+j*size, l_array+l*size), ++l;
            else
                assign(out+j*size, r_array+r*size), ++r;
        }
        tail += c;
        count -= c;
    } else if (tail) {
        t = i*k;
        r_array = array+t*size;

        memcpy(out+t*size, r_array, (count+tail-t)*size);
    }

    c *= 2;
    if (c < count) {
        pswap(&array, &out);
        goto merge;
    }

    if (tail) {
        if (count) {
            count += tail;
            l_array = out;
            r_array = out+(count-tail)*size;

            for (j=l=r=0;j<count;++j) {
                if (r >= tail || (l < c && cmp(l_array+l*size, r_array+r*size) == cmp_result_less))
                    assign(array+j*size, l_array+l*size), ++l;
                else
                    assign(array+j*size, r_array+r*size), ++r;
            }

            pswap(&array, &out);
        } else
            count += tail;
    }

    if (origin != out)
        memcpy(origin, out, count*size);
    else
        out = array;

    free(out);
}

void
merge_sort1(void *array,
        unsigned int count,
        unsigned int size,
        assign_t assign,
        swap_t swp,
        compare_t cmp)
{
    __merge_sort1_v0(array, count, size, assign, swp, cmp);
}

void
merge_sort2(void *array,
        unsigned int count,
        unsigned int size,
        assign_t assign,
        swap_t swp,
        compare_t cmp)
{
    __merge_sort2_v0(array, count, size, assign, swp, cmp);
}

void merge_sort1_parallel(void *array,
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
    __merge_sort1_v1(&sort_data);
}

#ifdef MSORT_MAIN
enum sort_variant {
    sort_variant_ms1,
    sort_variant_ms2
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
    fprintf(stderr, "%s --sort-variant|-s MS1|MS2 [--threads|-t <num>] [--input-data|-i <path>] [--count|-c <num>] [--output-data|-o <path>] [--dump]\n", prog);
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
                if (!strncmp(optarg, "MS1", 3))
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
        printf("\nOriginal array:\n");
        print_array(array, count);
    }

    switch (sort_variant) {
        case sort_variant_ms1:
            printf("MS1(%d threads)\n", threads);
            gettimeofday(&tb, NULL);
            if (threads)
                merge_sort1_parallel(array, count, threads, sizeof(unsigned int), uint_assign, uint_swap, uint_cmp);
            else
                merge_sort1(array, count, sizeof(unsigned int), uint_assign, uint_swap, uint_cmp);
            gettimeofday(&ta, NULL);
            break;

        case sort_variant_ms2:
            printf("MS2(%d threads)\n", threads);
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
