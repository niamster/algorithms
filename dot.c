#include <stdio.h>
#include <string.h>

#include "dot.h"
#include "sllist.h"

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

void dot_dump_begin(FILE *out,
                    const char *label)
{
    fprintf(out, "digraph %s {\n", label);
    fprintf(out, "    graph [center rankdir=LR]\n");
}

void dot_dump_end(FILE *out)
{
    fprintf(out, "}\n");
}

void dot_dump_table(FILE *out,
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

void dot_dump_table_link_to_list(FILE *out,
                                 const char *label,
                                 int id,
                                 const char *dst_label,
                                 int dst_id)
{
    fprintf(out, "    \"%s\":%s_%d -> %s_%d_0;\n", label, label, id, dst_label, dst_id);
}

void __dot_dump_sllist_header(FILE *out,
                              const char *label,
                              unsigned int idx)
{
    fprintf(out, "    subgraph %s%u {\n", label, idx);
}

void __dot_dump_sllist_footer(FILE *out)
{
    fprintf(out, "    }\n");
}

void __dot_dump_sllist_node(FILE *out,
                            const char *label,
                            unsigned int idx,
                            unsigned int id,
                            struct sllist *head,
                            struct sllist *node,
                            struct sllist *prev,
                            const char *name,
                            const char *value)
{
    char *escaped_name = escape_quotes(name);

    fprintf(out, "        %s_%u_%u [label=\"%s\"];\n", label, idx, id, escaped_name);
    if (prev != head) {
        fprintf(out, "        %s_%u_%u -> %s_%u_%u;\n", label, idx, id-1, label, idx, id);
    }
    if (node->next == head) {
        fprintf(out, "        %s_%u_%u -> %s_%u_%u;\n", label, idx, id, label, idx, 1);
    }

    free(escaped_name);
}
