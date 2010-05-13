#ifndef _DOT_H_
#define _DOT_H_

#include <stdio.h>

#include "sllist.h"
#include "helpers.h"

void dot_dump_begin(FILE *out,
                    char *name);
void dot_dump_end(FILE *out);

#define dot_dump_sllist(out, idx, container, list, name, value)         \
    do {                                                                \
        struct sllist *e, *p;                                           \
        unsigned int id = 0;                                            \
        __dot_dump_sllist_header(out, idx);                             \
        sllist_for_each_prev(&(container)->list, e, p) {                \
            __typeof__(*container) *node =                              \
                container_of(e, __typeof__(*container), list);          \
            __dot_dump_sllist_node(out,                                 \
                                   idx,                                 \
                                   ++id,                                \
                                   &(container)->list, e, p,            \
                                   node->name, node->value);            \
        }                                                               \
        __dot_dump_sllist_footer(out);                                  \
    } while (0)

void __dot_dump_sllist_header(FILE *out,
                              unsigned int idx);
void __dot_dump_sllist_footer(FILE *out);
void __dot_dump_sllist_node(FILE *out,
                            unsigned int idx,
                            unsigned int id,
                            struct sllist *head,
                            struct sllist *node,
                            struct sllist *prev,
                            const char *name,
                            const char *value);

#endif
