#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <pthread.h>
#include <errno.h>
#include <getopt.h>

static inline void
swap(unsigned int *x,
     unsigned int *y)
{
    if (*x != *y) {
        *x ^= *y;
        *y ^= *x;
        *x ^= *y;
    }
} __attribute__((always_inline));

int
generate_array(unsigned int **array,
               int *count,
               const char *path)
{
    int v;
    if ((v = open(path, O_RDONLY)) == -1) {
        fprintf(stderr, "Error error opening to %s: %s", path, strerror(errno));
        return -1;
    }
    if (*count == 0) {
        struct stat s;
        if (fstat(v, &s) == -1) {
            fprintf(stderr, "Error error getting information about %s: %s", path, strerror(errno));
            return -1;
        }
        *count = s.st_size/sizeof(unsigned int);
    }
    if (!(*array = malloc(*count*sizeof(unsigned int)))) {
        fprintf(stderr, "Error error allocating %d bytes: %s", *count*sizeof(unsigned int), strerror(errno));
        return -1;
    }
    if (read(v, *array, *count*sizeof(unsigned int)) < *count*sizeof(unsigned int)) {
        fprintf(stderr, "Error error reading %d bytes from %s: %s", *count*sizeof(unsigned int), path, strerror(errno));
        return -1;
    }
    close(v);

    for (v=0;v<*count;++v)
        (*array)[v] %= 10000;

    return 0;
}

void
print_array(unsigned int *array,
            int count)
{
    int i;
    printf("array of %d elements\n", count);

    for (i=0;i<count;++i) {
        if (i!=0 && !(i%16))
            printf("\n");
        printf("%4u ", array[i]);
    }
    printf("\n");
}

struct sort_data;
typedef void (*quick_sort_fn)(struct sort_data *);

struct sort_data {
    quick_sort_fn quick_sort;
    quick_sort_fn quick_sort_thread;
    unsigned int *array;
    int count;
    int max_threads;
    pthread_t thread;
};

void *
__quick_sort_parallel_wrapper(struct sort_data *data)
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
__quick_sort1(struct sort_data *data)
{
    struct sort_data sort_data[] = {
        [0] = {
            .quick_sort = data->quick_sort,
            .quick_sort_thread = __quick_sort_parallel,
        },
        [1] = {
            .quick_sort = data->quick_sort,
            .quick_sort_thread = __quick_sort_parallel,
        },
    };
    unsigned int *array = data->array;
    int count = data->count;
    int m = (count-1)/2;
    int l, r;
    unsigned int *l_array, *r_array;
    int l_count = 0, r_count = 0;
    unsigned int pivot;

    if (count <= 1)
        return;

    l_array = malloc(count*sizeof(unsigned int));
    r_array = malloc(count*sizeof(unsigned int));

    pivot = array[m];

    for (l=0;l<count;++l) {
        if (l == m)
            continue;
        if (array[l] <= array[m])
            l_array[l_count++] = array[l];
        else
            r_array[r_count++] = array[l];
    }

    sort_data[0].array = l_array;
    sort_data[0].count = l_count;
    sort_data[1].array = r_array;
    sort_data[1].count = r_count;
    if (data->quick_sort_thread && data->max_threads > 1) {
        data->max_threads -= 2;
        sort_data[0].max_threads = data->max_threads;
        sort_data[1].max_threads = data->max_threads;
        data->quick_sort_thread(&sort_data[0]);
        data->quick_sort_thread(&sort_data[1]);

        __quick_sort_parallel_join(&sort_data[0]);
        __quick_sort_parallel_join(&sort_data[1]);
    } else {
        (data->quick_sort)(&sort_data[0]);
        (data->quick_sort)(&sort_data[1]);
    }

    for (l=0;l<l_count;++l)
        array[l] = l_array[l];
    array[l++] = pivot;

    for (r=0;r<r_count;++l, ++r)
        array[l] = r_array[r];

    free(l_array);
    free(r_array);
}

/* void */
/* __quick_sort2_v0(unsigned int *array, */
/*                  int l, int r) */
/* { */
/*     int m = (l+r)/2; */
/*     int i, s; */
/*     unsigned int pivot; */

/*     if (l >= r) */
/*         return; */

/*     pivot = array[m]; */

/*     swap(&array[r], &array[m]); */

/*     for (s=i=l;i<r;++i) */
/*         if (array[i] <= pivot) { */
/*             if (s != i) */
/*                 swap(&array[s], &array[i]); */
/*             ++s; */
/*         } */
/*     swap(&array[s], &array[r]); */

/*     __quick_sort2_v0(array, l, s-1); */
/*     __quick_sort2_v0(array, s+1, r); */
/* } */

/* void */
/* quick_sort2_v0(unsigned int *array, */
/*                int count) */
/* { */
/*     __quick_sort2_v0(array, 0, count-1); */
/* } */

void
__quick_sort2_v1(struct sort_data *data)
{
    struct sort_data sort_data[] = {
        [0] = {
            .quick_sort = data->quick_sort,
            .quick_sort_thread = __quick_sort_parallel,
        },
        [1] = {
            .quick_sort = data->quick_sort,
            .quick_sort_thread = __quick_sort_parallel,
        },
    };
    unsigned int *array = data->array;
    int count = data->count;
    int l = 0, r = count-1;
    int m = (count-1)/2;
    int i, s;
    unsigned int pivot;

    if (l >= r)
        return;

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

    if (data->quick_sort_thread && data->max_threads > 1) {
        data->max_threads -= 2;
        sort_data[0].max_threads = data->max_threads;
        sort_data[1].max_threads = data->max_threads;
        data->quick_sort_thread(&sort_data[0]);
        data->quick_sort_thread(&sort_data[1]);

        __quick_sort_parallel_join(&sort_data[0]);
        __quick_sort_parallel_join(&sort_data[1]);
    } else {
        (data->quick_sort)(&sort_data[0]);
        (data->quick_sort)(&sort_data[1]);
    }
}

void
quick_sort2(unsigned int *array,
            int count)
{
    struct sort_data sort_data = {
        .quick_sort = __quick_sort2_v1,
        .array = array,
        .count = count
    };
    __quick_sort2_v1(&sort_data);
}

void
quick_sort1(unsigned int *array,
            int count)
{
    struct sort_data sort_data = {
        .quick_sort = __quick_sort1,
        .array = array,
        .count = count
    };
    __quick_sort1(&sort_data);
}

void quick_sort1_parallel(unsigned int *array,
                          int count,
                          int threads)
{
    struct sort_data sort_data = {
        .quick_sort = __quick_sort1,
        .quick_sort_thread = __quick_sort_parallel,
        .array = array,
        .count = count,
        .max_threads = threads,
    };
    __quick_sort1(&sort_data);
}

void quick_sort2_parallel(unsigned int *array,
                          int count,
                          int threads)
{
    struct sort_data sort_data = {
        .quick_sort = __quick_sort2_v1,
        .quick_sort_thread = __quick_sort_parallel,
        .array = array,
        .count = count,
        .max_threads = threads,
    };
    __quick_sort2_v1(&sort_data);
}

/*
  dd if=/dev/urandom of=/tmp/rnd count=1000000 bs=4
  ./qsort --sort-variant QS1 --input-data /tmp/rnd --output-data /tmp/QS1
  ./qsort --sort-variant QS1 --input-data /tmp/rnd --output-data /tmp/QS1P --threads 8
  ./qsort --sort-variant QS2 --input-data /tmp/rnd --output-data /tmp/QS2
  ./qsort --sort-variant QS2 --input-data /tmp/rnd --output-data /tmp/QS2P --threads 8
*/

enum sort_variant {
    QS1,
    QS2
};

int
usage(const char *prog)
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "%s --sort-variant|-s QS1|QS2 [--threads|-t <num>] [--input-data|-i <path>] [--count|-c <num>] [--output-data|-o <path>] [--print-arrays|-p]\n", prog);
    return 1;
}

int main(int argc, char **argv)
{
    struct timeval tb, ta;
    unsigned long secs, msecs, usecs;

    int print_arrays = 0;
    int sort_variant = -1;
    int count = 0;
    int threads = 0;
    const char *input_data = "/dev/random", *output_data = NULL;
    unsigned int *array;

    int opt;
    const struct option options[] = {
        {"sort-variant",  required_argument, NULL, 's'},
        {"threads",       required_argument, NULL, 't'},
        {"input-data",    required_argument, NULL, 'i'},
        {"output-data",   required_argument, NULL, 'o'},
        {"count",         required_argument, NULL, 'c'},
        {"print-arrays",  no_argument,       NULL, 'p'},
        {0,               0,                 0,    0}
    };

    while ((opt = getopt_long(argc, argv, "s:t:i:o:c:p", options, NULL)) != -1) {
        switch (opt) {
            case 's':
                if (!strncmp(optarg, "QS1", 3))
                    sort_variant = QS1;
                else if (!strncmp(optarg, "QS2", 3))
                    sort_variant = QS2;
                else
                    return usage(argv[0]);
                break;
            case 'p':
                print_arrays = 1;
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
            default:
                return usage(argv[0]);
        }
    }
    if (sort_variant == -1)
        return usage(argv[0]);

    if (generate_array(&array, &count, input_data) == -1)
        return 1;
    printf("Elements: %d\n", count);
    if (print_arrays)
        print_array(array, count);

    switch (sort_variant) {
        case QS1:
            printf("QS1(%d threads)\n", threads);
            gettimeofday(&tb, NULL);
            if (threads)
                quick_sort1_parallel(array, count, threads);
            else
                quick_sort1(array, count);
            gettimeofday(&ta, NULL);
            break;

        case QS2:
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

    if (print_arrays)
        print_array(array, count);

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
