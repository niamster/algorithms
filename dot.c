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
        if (*str == '"')
            *p++ = '\\';
        *p++ = *str++;
    }

    return new;
}

void dot_dump_begin(FILE *out,
                    char *name)
{
    fprintf(out, "graph %s {\n", name);
}

void dot_dump_end(FILE *out)
{
    fprintf(out, "}\n");
}

void __dot_dump_sllist_header(FILE *out,
                              unsigned int idx)
{
    fprintf(out, "    subgraph sllist%u {\n", idx);
}

void __dot_dump_sllist_footer(FILE *out)
{
    fprintf(out, "    }\n");
}

void __dot_dump_sllist_node(FILE *out,
                            unsigned int idx,
                            unsigned int id,
                            struct sllist *head,
                            struct sllist *node,
                            struct sllist *prev,
                            const char *name,
                            const char *value)
{
    char *escaped_name = escape_quotes(name);

    fprintf(out, "        E_%u_%u[label=\"%s\"];\n", idx, id, escaped_name);
    if (prev != head) {
        fprintf(out, "        E_%u_%u -- E_%u_%u;\n", idx, id-1, idx, id);
    }
    if (node->next == head) {
        fprintf(out, "        E_%u_%u -- E_%u_%u;\n", idx, id, idx, 1);
    }

    free(escaped_name);
}
