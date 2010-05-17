#ifndef _HELPERS_H_
#define _HELPERS_H_

int generate_array(unsigned int **array,
                   int *count,
                   const char *path);
void print_array(unsigned int *array,
                 int count);

#define KEY_LEN   8
#define VALUE_LEN 24

struct key_value {
    unsigned char key[KEY_LEN];
    unsigned char value[VALUE_LEN];
};

int generate_key_value(struct key_value **data,
                       int *count,
                       const char *path);
void print_key_value(struct key_value *data,
                     int count);

typedef enum {
    cmp_result_less     = -1,
    cmp_result_equal    = 0,
    cmp_result_greater  = 1
} cmp_result_t;

static inline void
swap(unsigned int *x, unsigned int *y)
{
    if (*x != *y) {
        *x ^= *y;
        *y ^= *x;
        *x ^= *y;
    }
}

static inline int
int_sign(int x)
{
    return (x > 0) - (x < 0);
}

#define container_of(ptr, type, member)                             \
    (type *)((char *)ptr - (unsigned int)(&((type *)0)->member))

#endif
