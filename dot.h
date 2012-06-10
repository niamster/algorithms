#ifndef _DOT_H_
#define _DOT_H_

#include <stdio.h>

#include "sllist.h"
#include "helpers.h"

typedef enum {
    dot_graph_direction_left_to_right,
    dot_graph_direction_right_to_left,
    dot_graph_direction_top_to_bottom,
    dot_graph_direction_bottom_to_top,
} dot_graph_direction_t;

void dot_dump_begin(FILE *out,
        const char *label,
        dot_graph_direction_t direction);
void dot_dump_end(FILE *out);

void dot_dump_table(FILE *out,
        const char *label,
        unsigned long size);

void dot_dump_link_table_to_sllist_head(FILE *out,
        const char *src_label,
        unsigned long src_id,
        const char *dst_label,
        unsigned long dst_id);

void dot_dump_link_table_to_node(FILE *out,
        const char *src_label,
        unsigned long src_id,
        const char *dst_label,
        unsigned long dst_id);

void dot_dump_link_node_to_node(FILE *out,
        const char *src_label,
        unsigned long src_id,
        const char *dst_label,
        unsigned long dst_id);

#define dot_dump_node(out, label, id, name)                             \
    dot_dump_shape_colored(out, label, id, name, "black", "black", NULL, NULL)


/* Acceptable node shapes:
   circle, box, triangle, invtriangle, ellipse, diamond, parallelogram*/
void dot_dump_shape_colored(FILE *out,
        const char *label,
        unsigned long id,
        const char *name,
        const char *txt_color,
        const char *edge_color,
        const char *bg_color,
        const char *shape);

#define dot_dump_sllist(out, label, idx, head, type, member, name)  \
    do {                                                            \
        struct sllist *e;                                           \
        unsigned long pos = 0;                                      \
        __dot_dump_header(out, label, idx);                         \
        sllist_for_each(head, e) {                                  \
            type *node =                                            \
                container_of(e, type, member);                      \
            __dot_dump_sllist_node(out,                             \
                    label,                                          \
                    idx,                                            \
                    pos++,                                          \
                    head, e,                                        \
                    node->name);                                    \
        }                                                           \
        __dot_dump_footer(out);                                     \
    } while (0)

void __dot_dump_header(FILE *out,
        const char *label,
        unsigned long idx);
void __dot_dump_footer(FILE *out);
void __dot_dump_sllist_node(FILE *out,
        const char *label,
        unsigned long idx,
        unsigned long id,
        struct sllist *head,
        struct sllist *node,
        const char *name);

#endif
