#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dot.h"
#include "sllist.h"

static char *dot_graph_directions[] = {
    [dot_graph_direction_left_to_right] = "LR",
    [dot_graph_direction_right_to_left] = "RL",
    [dot_graph_direction_top_to_bottom] = "TB",
    [dot_graph_direction_bottom_to_top] = "BT"
};

char *
escape_quotes(const char *str)
{
    unsigned long len = strlen(str) * 2 + 1;
    char *new = malloc(len), *p;

    if (new == NULL)
        return NULL;

    memset(new, 0x0, len);
    p = new;

    while (*str) {
        if (*str == '"' || *str == '\\')
            *p++ = '\\';
        *p++ = *str++;
    }

    return new;
}

void
dot_dump_begin(FILE *out,
        const char *label,
        dot_graph_direction_t direction)
{
    fprintf(out, "digraph %s {\n", label);
    fprintf(out, "    graph [center rankdir=%s]\n", dot_graph_directions[direction]);
}

void
dot_dump_end(FILE *out)
{
    fprintf(out, "}\n");
}

void
dot_dump_table(FILE *out,
        const char *label,
        int size)
{
    int i;

    fprintf(out, "    \"%s\" [\n", label);
    fprintf(out, "        label = \"");
    for (i=0;i<size-1;++i) {
        fprintf(out, " <%s_%d> %d |", label, i, i);
    }
    fprintf(out, " <%s_%d> %d\"\n", label, i, i);
    fprintf(out, "        shape = \"record\"\n");
    fprintf(out, "    ];\n");
}

void
dot_dump_link_table_to_sllist_head(FILE *out,
        const char *src_label,
        int src_id,
        const char *dst_label,
        int dst_id)
{
    fprintf(out, "    \"%s\":%s_%d -> %s_%d_0;\n", src_label, src_label, src_id, dst_label, dst_id);
}

void
dot_dump_link_table_to_node(FILE *out,
        const char *src_label,
        int src_id,
        const char *dst_label,
        int dst_id)
{
    fprintf(out, "    \"%s\":%s_%d -> %s_%d;\n", src_label, src_label, src_id, dst_label, dst_id);
}

void
dot_dump_link_node_to_node(FILE *out,
        const char *src_label,
        int src_id,
        const char *dst_label,
        int dst_id)
{
    fprintf(out, "    %s_%d -> %s_%d;\n", src_label, src_id, dst_label, dst_id);
}

void
dot_dump_shape_colored(FILE *out,
        const char *label,
        int id,
        const char *name,
        const char *txt_color,
        const char *edge_color,
        const char *bg_color,
        const char *shape)
{
    char *_name = escape_quotes(name);

    shape = shape?:"circle";

    if (bg_color)
        fprintf(out, "    %s_%d [label=\"%s\",fontcolor=\"%s\",color=\"%s\",fillcolor=\"%s\",style=\"filled\",shape=\"%s\"];\n",
                label, id, _name, txt_color, edge_color, bg_color, shape);
    else
        fprintf(out, "    %s_%d [label=\"%s\",fontcolor=\"%s\",color=\"%s\",shape=\"%s\"];\n",
                label, id, _name, txt_color, edge_color, shape);

    free(_name);
}

void
__dot_dump_header(FILE *out,
        const char *label,
        unsigned int idx)
{
    fprintf(out, "    subgraph %s%u {\n", label, idx);
}

void
__dot_dump_footer(FILE *out)
{
    fprintf(out, "    }\n");
}

void
__dot_dump_sllist_node(FILE *out,
        const char *label,
        unsigned int idx,
        unsigned int pos,
        struct sllist *head,
        struct sllist *node,
        const char *name)
{
    char *_name = escape_quotes(name);

    fprintf(out, "        %s_%u_%u [label=\"%s\"];\n", label, idx, pos, _name);
    if (pos > 0) {
        fprintf(out, "        %s_%u_%u -> %s_%u_%u;\n", label, idx, pos-1, label, idx, pos);
    }
    if (node->next == head) {
        fprintf(out, "        %s_%u_%u -> %s_%u_%u;\n", label, idx, pos, label, idx, 0);
    }

    free(_name);
}
