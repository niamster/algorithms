#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <stdio.h>

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
