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
