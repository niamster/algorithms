#ifndef _DOT_H_
#define _DOT_H_

#include <stdio.h>

#include "sllist.h"
#include "helpers.h"

void dot_dump_begin(FILE *out,
                    const char *label);
void dot_dump_end(FILE *out);

void dot_dump_table(FILE *out,
                    const char *label,
                    int size);
void dot_dump_table_link_to_list(FILE *out,
                                 const char *label,
                                 int id,
                                 const char *dst_label,
                                 int dst_id);

#define dot_dump_sllist(out, label, idx, head, type, member, name)      \
    do {                                                                \
        struct sllist *e;                                               \
        unsigned int pos = 0;                                           \
        __dot_dump_header(out, label, idx);                             \
        sllist_for_each(head, e) {                                      \
            type *node =                                                \
                container_of(e, type, member);                          \
            __dot_dump_sllist_node(out,                                 \
                                   label,                               \
                                   idx,                                 \
                                   pos++,                               \
                                   head, e,                             \
                                   node->name);                         \
        }                                                               \
        __dot_dump_footer(out);                                         \
    } while (0)

void __dot_dump_header(FILE *out,
                       const char *label,
                       unsigned int idx);
void __dot_dump_footer(FILE *out);
void __dot_dump_sllist_node(FILE *out,
                            const char *label,
                            unsigned int idx,
                            unsigned int id,
                            struct sllist *head,
                            struct sllist *node,
                            const char *name);

#endif
