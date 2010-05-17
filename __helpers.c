#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>

#include "helpers.h"

int
generate_array(unsigned int **array,
               int *count,
               const char *path)
{
    int v;
    if ((v = open(path, O_RDONLY)) == -1) {
        fprintf(stderr, "Error error opening to %s: %s", path, strerror(errno));
        return -1;
    }
    if (*count == 0) {
        struct stat s;
        if (fstat(v, &s) == -1) {
            fprintf(stderr, "Error error getting information about %s: %s", path, strerror(errno));
            return -1;
        }
        *count = s.st_size/sizeof(unsigned int);
    }
    if (!(*array = malloc(*count*sizeof(unsigned int)))) {
        fprintf(stderr, "Error error allocating %d bytes: %s", *count*sizeof(unsigned int), strerror(errno));
        return -1;
    }
    if (read(v, *array, *count*sizeof(unsigned int)) < *count*sizeof(unsigned int)) {
        fprintf(stderr, "Error error reading %d bytes from %s: %s", *count*sizeof(unsigned int), path, strerror(errno));
        return -1;
    }
    close(v);

    for (v=0;v<*count;++v)
        (*array)[v] %= 10000;

    return 0;
}

void
print_array(unsigned int *array,
            int count)
{
    int i;
    printf("array of %d elements\n", count);

    for (i=0;i<count;++i) {
        if (i!=0 && !(i%16))
            printf("\n");
        printf("%4u ", array[i]);
    }
    printf("\n");
}

int
generate_key_value(struct key_value **data,
              int *count,
              const char *path)
{
    int v;
    if ((v = open(path, O_RDONLY)) == -1) {
        fprintf(stderr, "Error error opening to %s: %s", path, strerror(errno));
        return -1;
    }
    if (*count == 0) {
        struct stat s;
        if (fstat(v, &s) == -1) {
            fprintf(stderr, "Error error getting information about %s: %s", path, strerror(errno));
            return -1;
        }
        *count = s.st_size/sizeof(struct key_value);
    }
    if (!(*data = malloc(*count*sizeof(struct key_value)))) {
        fprintf(stderr, "Error error allocating %d bytes: %s", *count*sizeof(struct key_value), strerror(errno));
        return -1;
    }
    if (read(v, *data, *count*sizeof(struct key_value)) < *count*sizeof(struct key_value)) {
        fprintf(stderr, "Error error reading %d bytes from %s: %s", *count*sizeof(struct key_value), path, strerror(errno));
        return -1;
    }
    close(v);

    for (v=0;v<*count;++v) {
        unsigned char *d;

        d = (*data)[v].key;
        d[KEY_LEN-1] = '\0';
        while (*d) {
            if (*d >= 0x80)
                *d -= 0x80;
            if (*d < 0x20)
                *d += 0x20;
            else if (*d > 0x7e)
                *d -= 0x7e - 0x20;
            ++d;
        }

        d = (*data)[v].value;
        d[VALUE_LEN-1] = '\0';
        while (*d) {
            if (*d >= 0x80)
                *d -= 0x80;
            if (*d < 0x20)
                *d += 0x20;
            else if (*d > 0x7e)
                *d -= 0x7e - 0x20;
            ++d;
        }
    }

    return 0;
}

void
print_key_value(struct key_value *data,
                int count)
{
    printf("data of %d elements\n", count);
}
