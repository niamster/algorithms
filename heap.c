#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/time.h>
#include <errno.h>
#include <getopt.h>

#include "helpers.h"
#include "heap.h"

void
__heap_push_up(void *array,
        unsigned int e,
        unsigned int count,
        unsigned int size,
        swap_t swp,
        compare_t cmp)
{
    unsigned int p, l, r;
    unsigned int m;

    for (;;) {
        if (e == 0)
            break;
        p = (e-1)/2;
        l = 2*p + 1;
        r = 2*p + 2;
        m = p;
        if (l < count && cmp(array+l*size, array+m*size) == cmp_result_greater)
            m = l;
        if (r < count && cmp(array+r*size, array+m*size) == cmp_result_greater)
            m = r;
        if (m == p)
            break;
        swp(array+m*size, array+p*size);
        e = p;
    }
}

void
__heap_push_down(void *array,
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

void
__heap_build(void *array,
        unsigned int count,
        unsigned int size,
        swap_t swp,
        compare_t cmp)
{
    // travers all parent nodes
    long m = (count-1)/2;
    for (;m>=0;--m)
        __heap_push_down(array, m, count, size, swp, cmp);
}

#ifdef HEAP_MAIN
static void
build_heap(struct heap *heap, unsigned int *array, unsigned int count)
{
    unsigned int i;

    for (i=0;i<count;++i)
        if (!heap_push(heap, &array[i])) {
            fprintf(stderr, "Failed to push element(%u) %u\n", i, array[i]);
            break;
        }
}

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
    return (cmp_result_t)int_sign(*(unsigned int *)b - *(unsigned int *)a);
}

static int
usage(const char *prog)
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "%s [--input-data|-i <path>] [--count|-c <num>] [--dump]\n", prog);
    return 1;
}

int main(unsigned int argc, char **argv)
{
    struct timeval tb, ta;
    unsigned long secs, msecs, usecs;

    int dump = 0;
    unsigned int count = 0;
    const char *input_data = "/dev/random";
    unsigned int *array;
    struct heap heap;

    int opt;
    const struct option options[] = {
        {"input-data",    required_argument, NULL, 'i'},
        {"count",         required_argument, NULL, 'c'},
        {"dump",          no_argument,       &dump, 1},
        {0,               0,                 0,    0}
    };

    while ((opt = getopt_long(argc, argv, "s:t:i:o:c:", options, NULL)) != -1) {
        switch (opt) {
            case 'c':
                count = atoi(optarg);
                break;
            case 'i':
                input_data = optarg;
                break;
            case 0:
                break;
            default:
                return usage(argv[0]);
        }
    }

    if (generate_array(&array, &count, input_data) == -1)
        return 1;

    printf("Elements: %d\n", count);
    if (dump) {
        printf("Original array:\n");
        print_array(array, count);
    }

    if (!heap_init(&heap, count, sizeof(unsigned int), uint_assign, uint_swap, uint_cmp)) {
        printf("Failed to initialize heap\n");
        goto out;
    }

    gettimeofday(&tb, NULL);
    build_heap(&heap, array, count);
    gettimeofday(&ta, NULL);

    usecs = ta.tv_sec*1000000 + ta.tv_usec - tb.tv_sec*1000000 - tb.tv_usec;
    secs = usecs/1000000;
    usecs %= 1000000;
    msecs = usecs/1000;
    usecs %= 1000;
    printf("Heap build time: %lu seconds %lu msecs %lu usecs\n", secs, msecs, usecs);

    {
        unsigned int *el;

        el = heap_get_top(&heap);
        if (!el) {
            fprintf(stderr, "Failed to get heap top\n");
            goto out_heap;
        }
        printf("Heap top: %u\n", *el);

        el = heap_get_bottom(&heap);
        if (!el) {
            fprintf(stderr, "Failed to get heap bottom\n");
            goto out_heap;
        }
        printf("Heap bottom: %u\n", *el);
    }

    {
        unsigned int top, el;
        unsigned n;

        if (!heap_pop_top(&heap, &top)) {
            fprintf(stderr, "Failed to get heap top\n");
            goto out_heap;
        }
        n = 1;
        while (heap_pop_top(&heap, &el)) {
            ALGO_ASSERT("heap", top <= el, "heap is not ordered");
            top = el;
            ++n;
        }
        ALGO_ASSERT("heap", n == count, "wrong number of elements poped");
    }

    if (dump) {
        unsigned int el;
        int i;

        build_heap(&heap, array, count);

        i = 0;
        printf("\nPopping from heap top:\n");
        while (heap_pop_top(&heap, &el)) {
            if (i!=0 && !(i%16))
                printf("\n");
            printf("%4u ", el);
            ++i;
        }
        printf("\n");

        build_heap(&heap, array, count);

        i = 0;
        printf("\nPopping from heap bottom:\n");
        while (heap_pop_bottom(&heap, &el)) {
            if (i!=0 && !(i%16))
                printf("\n");
            printf("%4u ", el);
            ++i;
        }
        printf("\n");
    }

  out_heap:
    heap_destroy(&heap);
  out:
    free(array);

    return 0;
}
#endif
