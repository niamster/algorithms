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
typedef void (*merge_sort_fn)(struct sort_data *);

struct sort_data {
    /* Sort callbacks */
    merge_sort_fn merge_sort;

    /* Array to sort */
    unsigned int *array;
    unsigned int count;

    unsigned int max_threads;
    pthread_t thread;
};

void *__merge_sort_parallel_wrapper(struct sort_data *data)
{
    data->merge_sort(data);

    return NULL;
}

void __merge_sort_parallel(struct sort_data *data)
{
    pthread_create(&data->thread, NULL, (void *(*)(void*))__merge_sort_parallel_wrapper, data);
}

void __merge_sort_parallel_join(struct sort_data *data)
{
    pthread_join(data->thread, NULL);
}

void
__merge_sort1_v0(unsigned int *array,
                 unsigned int count)
{
    unsigned int m = (count-1)/2;
    unsigned int i, l, r;
    unsigned int *l_array, *r_array;

    if (count <= 1)
        return;

    if (count == 2) {
        if (array[0] > array[1])
            swap(&array[0], &array[1]);

        return;
    }

    l_array = malloc(m*sizeof(unsigned int));
    r_array = malloc((count-m)*sizeof(unsigned int));

    for (i=0;i<m;++i)
        l_array[i] = array[i];

    for (r=0;i<count;++i,++r)
        r_array[r] = array[i];

    __merge_sort1_v0(l_array, m);
    __merge_sort1_v0(r_array, count-m);

    for (i=0,l=0,r=0;i<count;++i) {
        if (r >= (count - m) || (l < m && l_array[l] < r_array[r]))
            array[i] = l_array[l++];
        else
            array[i] = r_array[r++];
    }

    free(l_array);
    free(r_array);
}

void
__merge_sort1_v1(struct sort_data *data)
{
    struct sort_data sort_data[] = {
        [0] = {
            .merge_sort = __merge_sort1_v1,
        },
        [1] = {
            .merge_sort = __merge_sort1_v1,
        },
    };
    unsigned int *array = data->array;
    unsigned int count = data->count;
    unsigned int m = (count-1)/2;
    unsigned int i, l, r;
    unsigned int *l_array, *r_array;

    if (count <= 1)
        return;

    if (count == 2) {
        if (array[0] > array[1])
            swap(&array[0], &array[1]);

        return;
    }

    l_array = malloc(m*sizeof(unsigned int));
    r_array = malloc((count-m)*sizeof(unsigned int));

    for (i=0;i<m;++i)
        l_array[i] = array[i];

    for (r=0;i<count;++i,++r)
        r_array[r] = array[i];

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
        if (r >= (count - m) || (l < m && l_array[l] < r_array[r]))
            array[i] = l_array[l++];
        else
            array[i] = r_array[r++];
    }

    free(l_array);
    free(r_array);
}

void
__merge_sort2_v0(unsigned int *array,
        unsigned int count)
{
    unsigned int chunks = count/2;
    unsigned int tail = count%2;
    unsigned int *origin = array;
    unsigned int *out;
    unsigned int i, j, k, t, c;
    unsigned int l, r;
    unsigned int *l_array, *r_array;

    if (count <= 1)
        return;

    if (count == 2) {
        if (array[0] > array[1])
            swap(&array[0], &array[1]);

        return;
    }

    for (i=0;i<chunks;++i) {
        t = i*2;
        if (array[t] > array[t+1])
            swap(&array[t], &array[t+1]);
    }

    out = malloc(count*sizeof(unsigned int));
    if (!out)
        return;

    count -= tail;
    c = 2;
  merge:
    k = c*2;

    for (i=0;i<count/k;++i) {
        t = i*k;
        l_array = array+t;
        r_array = &array[t+c];

        for (j=t,r=l=0;j<t+k;++j) {
            if (r >= c || (l < c && l_array[l] < r_array[r]))
                out[j] = l_array[l++];
            else
                out[j] = r_array[r++];
        }
    }

    if (count%k) {
        t = i*k;
        l_array = array+t;
        r_array = &array[t+c];

        for (j=t,r=l=0;j<count+tail;++j) {
            if (r >= tail || (l < c && l_array[l] < r_array[r]))
                out[j] = l_array[l++];
            else
                out[j] = r_array[r++];
        }
        tail += c;
        count -= c;
    } else if (tail) {
        t = i*k;
        r_array = array+t;

        for (j=t,r=0;j<count+tail;++j,++r)
            out[j] = r_array[r];
    }

    c *= 2;
    if (c < count) {
        pswap((void *)&array, (void *)&out);
        goto merge;
    }

    if (tail) {
        if (count) {
            count += tail;
            l_array = out;
            r_array = &out[count-tail];

            for (j=l=r=0;j<count;++j) {
                if (r >= tail || (l < c && l_array[l] < r_array[r]))
                    array[j] = l_array[l++];
                else
                    array[j] = r_array[r++];
            }

            pswap((void *)&array, (void *)&out);
        } else
            count += tail;
    }

    if (origin != out)
        memcpy(origin, out, count*sizeof(unsigned int));
    else
        out = array;

    free(out);
}

void
merge_sort1(unsigned int *array,
            unsigned int count)
{
    __merge_sort1_v0(array, count);
}

void
merge_sort2(unsigned int *array,
            unsigned int count)
{
    __merge_sort2_v0(array, count);
}

void merge_sort1_parallel(unsigned int *array,
                          unsigned int count,
                          unsigned int threads)
{
    struct sort_data sort_data = {
        .array = array,
        .count = count,
        .max_threads = threads,
    };
    __merge_sort1_v1(&sort_data);
}

#ifdef MSORT_MAIN
enum sort_variant {
    sort_variant_ms1,
    sort_variant_ms2
};

int
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
                merge_sort1_parallel(array, count, threads);
            else
                merge_sort1(array, count);
            gettimeofday(&ta, NULL);
            break;

        case sort_variant_ms2:
            printf("MS2(%d threads)\n", threads);
            gettimeofday(&tb, NULL);
            merge_sort2(array, count);
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
