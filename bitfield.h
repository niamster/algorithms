#ifndef _BITFIELD_H_
#define _BITFIELD_H_

#include <limits.h>
#include <string.h>
#include <stdlib.h>

#include "helpers.h"

#define BITFIELD_ASSERT(x, d) ALGO_ASSERT("bitfield", x, d)

#if __WORDSIZE >= 32
#if defined(__GNUC__)
#define bitfield_popcount __builtin_popcountl
#else
#define bitfield_popcount __bitfield_popcount
#endif
typedef unsigned long word_t;
#elif __WORDSIZE == 16
#if defined(__GNUC__)
#define bitfield_popcount __builtin_popcount
#else
#define bitfield_popcount __bitfield_popcount
#endif
typedef unsigned int word_t;
#if defined(__GNUC__)
#define bitfield_popcount __builtin_popcount
#else
#define bitfield_popcount __bitfield_popcount
#endif
#elif__WORDSIZE == 8
typedef unsigned char word_t;
#if defined(__GNUC__)
#define bitfield_popcount __builtin_popcount
#else
#define bitfield_popcount __bitfield_popcount
#endif
#else
#error Unknown CPU word size
#endif

typedef word_t bitset_t;

struct bitfield {
    word_t size;
    bitset_t field[];
};

typedef struct bitfield * bitfield_t;

#define __BS_SIZE               (sizeof(bitset_t)*8)
#define __BF_SIZE(sz)           ((sz)/__BS_SIZE + ((sz)%__BS_SIZE?1:0))

#define EMBEDDED_BITFIELD(name, sz)                         \
    struct {                                                \
        word_t ____________ ## name;                        \
        bitset_t ________ ## name[__BF_SIZE(sz)];           \
    } ________ ## name;                                     \
    bitfield_t name

#define EMBEDDED_BITFIELD_INITIALIZE(container, name, sz) do {          \
        (container)->name = (bitfield_t)&(container)->________ ## name; \
        (container)->name->size = (sz);                                 \
    } while (0)

#define BITFIELD(name, sz)                                  \
    struct {                                                \
        word_t ____________ ## name;                        \
        bitset_t ________ ## name[__BF_SIZE(sz)];           \
    } ________________ ## name = {sz};                      \
    bitfield_t name = (bitfield_t)&________________ ## name

#define STATIC_BITFIELD(name, sz)                           \
    static struct {                                         \
        word_t ____________ ## name;                        \
        bitset_t ________ ## name[__BF_SIZE(sz)];           \
    } ________________ ## name = {sz};                      \
    static bitfield_t name = (bitfield_t)&________________ ## name

#define __bitfield_bounds_test(call, bf, bit, ...) ({               \
            BITFIELD_ASSERT((bit) < (bf)->size, "bounds breach");   \
            call(bf, bit, ##__VA_ARGS__);                           \
        })

#if !defined(__GNUC__)
#warning slow popcount implementation
static inline unsigned int
__bitfield_popcount(bitset_t bs)
{
    unsigned int cnt = 0;

    while (bs) {
        if (bs&1)
            ++cnt;
        bs >>= 1;
    }

    return cnt;
}
#endif

static inline bitfield_t
bitfield_alloc(unsigned int size)
{
    bitfield_t bf;

    bf = malloc(sizeof(struct bitfield) + __BF_SIZE(size)*sizeof(bitset_t));
    if (bf)
        bf->size = size;

    return bf;
}

static inline void
bitfield_free(bitfield_t bf)
{
    free(bf);
}

#define bitfield_set(bf, bit, value) __bitfield_bounds_test(__bitfield_set, bf, bit, value)
static inline void
__bitfield_set(bitfield_t bf, unsigned int bit, int value)
{
    bf->field[bit/__BS_SIZE] &= ~((word_t)1 << (bit%__BS_SIZE));
    bf->field[bit/__BS_SIZE] |= (word_t)(value&1) << (bit%__BS_SIZE);
}

#define bitfield_set_one(bf, bit) __bitfield_bounds_test(__bitfield_set_one, bf, bit)
static inline void
__bitfield_set_one(bitfield_t bf, unsigned int bit)
{
    bf->field[bit/__BS_SIZE] |= (word_t)1 << (bit%__BS_SIZE);
}

#define bitfield_set_zero(bf, bit) __bitfield_bounds_test(__bitfield_set_zero, bf, bit)
static inline void
__bitfield_set_zero(bitfield_t bf, unsigned int bit)
{
    bf->field[bit/__BS_SIZE] &= ~((word_t)1 << (bit%__BS_SIZE));
}

#define bitfield_toggle(bf, bit) __bitfield_bounds_test(__bitfield_toggle, bf, bit)
static inline void
__bitfield_toggle(bitfield_t bf, unsigned int bit)
{
    bf->field[bit/__BS_SIZE] ^= (word_t)1 << (bit%__BS_SIZE);
}

#define bitfield_get(bf, bit) __bitfield_bounds_test(__bitfield_get, bf, bit)
static inline int
__bitfield_get(bitfield_t bf, unsigned int bit)
{
    return (bf->field[bit/__BS_SIZE] >> (bit%__BS_SIZE))&1;
}

static inline void
bitfield_set_all_zero(bitfield_t bf)
{
    memset(bf->field, 0, __BF_SIZE(bf->size)*sizeof(bitset_t));
}

static inline void
bitfield_set_all_one(bitfield_t bf)
{
    const unsigned int __bf_sz_words = __BF_SIZE(bf->size);
    const unsigned int __bf_sz_bytes = __bf_sz_words*sizeof(bitset_t);
    const unsigned int __bf_sz_bits = __bf_sz_bytes*8;

    memset(bf->field, ~0, __bf_sz_bytes);
    bf->field[__bf_sz_words-1] &= (word_t)(~0ULL) >> (__bf_sz_bits - bf->size);
}

static inline unsigned int
bitfield_count_ones(bitfield_t bf)
{
    unsigned int i, cnt = 0;

    for (i=0;i<__BF_SIZE(bf->size);++i)
        cnt += bitfield_popcount(bf->field[i]);

    return cnt;
}

static inline unsigned int
bitfield_count_zeros(bitfield_t bf)
{
    return bf->size - bitfield_count_ones(bf);
}
#endif
