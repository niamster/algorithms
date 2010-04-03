
#ifndef _HELPERS_H_
#define _HELPERS_H_

int generate_array(unsigned int **array, int *count, const char *path);

void print_array(unsigned int *array, int count);

static inline void
swap(unsigned int *x, unsigned int *y)
{
    if (*x != *y) {
        *x ^= *y;
        *y ^= *x;
        *x ^= *y;
    }
} __attribute__((always_inline));

#endif
