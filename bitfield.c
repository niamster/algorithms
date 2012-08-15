#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "bitfield.h"

struct s {
    EMBEDDED_BITFIELD(bf, 10);
};

BITFIELD(bf, 1);

int main(int argc, char **argv)
{
    struct s s;
    EMBEDDED_BITFIELD_INITIALIZE(&s, bf, 10);
    BITFIELD(bf0, 1);
    BITFIELD(bf1, 128);
    BITFIELD(bf2, 24);
    bitfield_t bf3 = bitfield_alloc(0);
    bitfield_t bf4 = bitfield_alloc(300);

    BITFIELD_ASSERT(bf3, "unable to allocate bitfield on heap");
    BITFIELD_ASSERT(bf4, "unable to allocate bitfield on heap");

    bitfield_set_one(bf, 0);
    BITFIELD_ASSERT(bitfield_get(bf, 0), "bit.0 is low");

    bitfield_set_one(s.bf, 0);
    BITFIELD_ASSERT(bitfield_get(s.bf, 0), "bit.0 is low");

    bitfield_set_one(bf0, 0);
    BITFIELD_ASSERT(bitfield_get(bf0, 0), "bit.0 is low");

    bitfield_set_one(bf1, 10);
    BITFIELD_ASSERT(bitfield_get(bf1, 10), "bit.10 is low");
    bitfield_set_zero(bf1, 10);
    BITFIELD_ASSERT(!bitfield_get(bf1, 10), "bit.10 is high");
    bitfield_toggle(bf1, 10);
    BITFIELD_ASSERT(bitfield_get(bf1, 10), "bit.10 not toggled");

    bitfield_set_one(bf2, 1);
    BITFIELD_ASSERT(bitfield_get(bf2, 1), "bit.1 is low");

    bitfield_set_all_zero(bf1);

    BITFIELD_ASSERT(bitfield_get(bf2, 1), "bit.1 is low");

    //bitfield_set_one(bf3, 0);

    bitfield_set_one(bf4, 100);
    BITFIELD_ASSERT(bitfield_get(bf4, 100), "bit.100 is low");

    bitfield_set_one(bf4, 200);

    BITFIELD_ASSERT(bitfield_count_ones(bf4) == 2, "wrong number of bits set high");
    BITFIELD_ASSERT(bitfield_count_zeros(bf4) == 298, "wrong number of bits set low");

    bitfield_set_all_one(bf4);

    BITFIELD_ASSERT(bitfield_count_ones(bf4) == 300, "wrong number of bits set high");
    BITFIELD_ASSERT(bitfield_count_zeros(bf4) == 0, "wrong number of bits set low");

    bitfield_set_all_zero(bf4);

    BITFIELD_ASSERT(bitfield_count_ones(bf4) == 0, "wrong number of bits set high");
    BITFIELD_ASSERT(bitfield_count_zeros(bf4) == 300, "wrong number of bits set low");

    bitfield_free(bf3);
    bitfield_free(bf4);

    return 0;
}
