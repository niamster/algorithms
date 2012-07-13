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
