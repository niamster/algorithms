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
    return (cmp_result_t)int_sign(*(unsigned int *)a - *(unsigned int *)b);
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

    if (dump) {
        unsigned int el;
        int i;

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
