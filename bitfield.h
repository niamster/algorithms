#ifndef _BITFIELD_H_
#define _BITFIELD_H_

#include <limits.h>

#if __WORDSIZE >= 32
typedef unsigned long bitset_t;
#elif __WORDSIZE == 16
typedef unsigned int bitset_t;
#elif__WORDSIZE == 8
typedef unsigned char bitset_t;
#else
#error Unknown CPU word size
#endif
typedef bitset_t bitfield_t [];

#define __BS_SIZE               (sizeof(bitset_t)*8)
#define __BF_SIZE(bits)         ((bits)/__BS_SIZE + ((bits)%__BS_SIZE?1:0))

#define BITFIELD(name, bits)    bitset_t name[__BF_SIZE(bits)]

static inline void
bitfield_set(bitfield_t bf, unsigned int bit, int value)
{
    bf[bit/__BS_SIZE] &= ~(1 << (bit%__BS_SIZE));
    bf[bit/__BS_SIZE] |= (value&1) << (bit%__BS_SIZE);
}

static inline void
bitfield_set_one(bitfield_t bf, unsigned int bit)
{
    bf[bit/__BS_SIZE] |= 1 << (bit%__BS_SIZE);
}

static inline void
bitfield_set_zero(bitfield_t bf, unsigned int bit)
{
    bf[bit/__BS_SIZE] &= ~(1 << (bit%__BS_SIZE));
}

static inline void
bitfield_toggle(bitfield_t bf, unsigned int bit)
{
    bf[bit/__BS_SIZE] ^= 1 << (bit%__BS_SIZE);
}

static inline int
bitfield_get(bitfield_t bf, unsigned int bit)
{
    return (bf[bit/__BS_SIZE] >> (bit%__BS_SIZE))&1;
}
#endif
