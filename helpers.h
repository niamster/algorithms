#ifndef _HELPERS_H_
#define _HELPERS_H_

#include <unistd.h>
#include <stdio.h>

#define ALGO_ASSERT(a, x, d) do {                                       \
        if (!(x)) {                                                     \
            fprintf(stderr,                                             \
                    a ":%s:%d: ASSERT(%s) failed: %s\n",                \
                    __FILE__, __LINE__, #x, d);                         \
            _exit(1);                                                   \
        }                                                               \
    } while (0)

unsigned int get_random(void);

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

#define __CONCAT_IMPL(x,y)  x##y
#define CONCAT_IMPL(x,y)    __CONCAT_IMPL(x,y)

#define UNIQUE_NAME_IMPL(name) CONCAT_IMPL(name,__COUNTER__)

#define ALIGN_IMPL(alignment) char UNIQUE_NAME_IMPL(pad)[0] __attribute__((aligned(alignment)))

/* #define L1_CACHE_LINE_SIZE _SC_LEVEL1_DCACHE_LINESIZE */
#define L1_CACHE_LINE_SIZE  64
#define L1_ALIGNED          ALIGN_IMPL(L1_CACHE_LINE_SIZE)

typedef enum {
    cmp_result_less     = -1,
    cmp_result_equal    = 0,
    cmp_result_greater  = 1
} cmp_result_t;

static inline const char *
stringify_cmp_result(cmp_result_t res) {
    static const char *cmp_result_str[] = {
        "less",
        "equal",
        "greater"
    };

    return cmp_result_str[res+1];
}

#define max(a, b) ({                                    \
            typeof(a) __a = (a);                        \
            typeof(b) __b = (b);                        \
            __a > __b?__a:__b;})

#define min(a, b) max(b, a)

static inline void
swap(unsigned int *x, unsigned int *y)
{
    if (*x != *y) {
        *x ^= *y;
        *y ^= *x;
        *x ^= *y;
    }
}

static inline void
pswap(void **x, void **y)
{
    void *t;

    t = *x;
    *x = *y;
    *y = t;
}

static inline int
int_sign(int x)
{
    return (x >> 31) | ((unsigned int)(-x) >> 31);
}

#define container_of(ptr, type, member)                             \
    (type *)((char *)ptr - (size_t)(&((type *)0)->member))


/* fls - find last (most-significant) bit set */
static inline int
fls(int x)
{
	int r = 32;

	if (!x)
		return 0;
	if (!(x & 0xffff0000u)) {
		x <<= 16;
		r -= 16;
	}
	if (!(x & 0xff000000u)) {
		x <<= 8;
		r -= 8;
	}
	if (!(x & 0xf0000000u)) {
		x <<= 4;
		r -= 4;
	}
	if (!(x & 0xc0000000u)) {
		x <<= 2;
		r -= 2;
	}
	if (!(x & 0x80000000u)) {
		x <<= 1;
		r -= 1;
	}
	return r;
}

#if __WORDSIZE == 64
static inline int
fls64(unsigned long x)
{
    int num = 64 - 1;

	if (!x)
		return 0;
	if (!(x & (~0ul << 32))) {
		num -= 32;
		x <<= 32;
	}
	if (!(x & (~0ul << (64-16)))) {
		num -= 16;
		x <<= 16;
	}
	if (!(x & (~0ul << (64-8)))) {
		num -= 8;
		x <<= 8;
	}
	if (!(x & (~0ul << (64-4)))) {
		num -= 4;
		x <<= 4;
	}
	if (!(x & (~0ul << (64-2)))) {
		num -= 2;
		x <<= 2;
	}
	if (!(x & (~0ul << (64-1))))
		num -= 1;

	return num + 1;
}
#endif

static inline unsigned
fls_long(unsigned long l)
{
#if __WORDSIZE == 64
    return fls64(l);
#else
    return fls(l);
#endif
}

/*
 * round up to nearest power of two
 */
static inline unsigned long
roundup_pow_of_two(unsigned long n)
{
	return 1UL << fls_long(n - 1);
}

/* ilog2 - log of base 2 of 32-bit or a 64-bit unsigned value */
static inline int
ilog2(unsigned long n)
{
	return fls_long(n) - 1;
}

#endif
