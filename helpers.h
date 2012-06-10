#ifndef _HELPERS_H_
#define _HELPERS_H_

unsigned int generate_random(void);

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
