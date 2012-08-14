#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "helpers.h"
#include "qsort.h"

int
binary_search(unsigned int *array,
        unsigned int count,
        unsigned int needle)
{
	unsigned int p = count >>= 1;
    unsigned int shift;

	while (count) {
		if (array[p] == needle)
			return p;

        shift = (count + 1) >> 1;

		if (needle > array[p]) {
			p += shift;
        } else {
			p -= shift;
        }
        count >>= 1;
	}

    if (array[p] == needle)     /* One element case */
        return p;

	return -1;
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

int
usage(const char *prog)
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "%s [--input-data|-i <path>] [--count|-c <num>] [--dump]\n", prog);
    return 1;
}

int main(int argc, char **argv)
{
    int dump = 0;
    unsigned int count = 0;
    const char *input_data = "/dev/random";
    unsigned int *array;

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
        printf("\nOriginal array:\n");
        print_array(array, count);
    }

    quick_sort1(array, count, sizeof(unsigned int), uint_assign, uint_swap, uint_cmp);

    if (dump) {
        printf("\nSorted array:\n");
        print_array(array, count);
    }

    {
        unsigned int i;
        for (i=0;i<count;++i) {
            int pos = binary_search(array, count, array[i]);
            if ((unsigned int)pos == -1 || array[i] != array[pos]) {
                fprintf(stderr, "Element %u(%u) was not found. Index returned: %d\n", i, array[i], pos);
                goto out;
            }
        }
        printf("All elements of array were successfully found\n");
    }

  out:
    free(array);

	return 0;
}
