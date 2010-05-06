#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <pthread.h>
#include <errno.h>
#include <getopt.h>

#define KEY_LEN   8
#define VALUE_LEN 24

struct hash_data {
    unsigned char key[KEY_LEN];
    unsigned char value[VALUE_LEN];
};

typedef int (*hash_function_t)(const char *, int limit);

struct hash_node {
    struct hash_data *data;
    struct hash_node *next;
};

int
generate_data(struct hash_data **data,
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
        *count = s.st_size/sizeof(struct hash_data);
    }
    if (!(*data = malloc(*count*sizeof(struct hash_data)))) {
        fprintf(stderr, "Error error allocating %d bytes: %s", *count*sizeof(struct hash_data), strerror(errno));
        return -1;
    }
    if (read(v, *data, *count*sizeof(struct hash_data)) < *count*sizeof(struct hash_data)) {
        fprintf(stderr, "Error error reading %d bytes from %s: %s", *count*sizeof(struct hash_data), path, strerror(errno));
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
print_data(struct hash_data *data,
           int count)
{
    printf("data of %d elements\n", count);
}

void
print_htable_entries(struct hash_node *node)
{
    while (node) {
        printf("    '%s' => '%s'\n", node->data->key, node->data->value);
        node = node->next;
    }
}

void
print_htable(struct hash_node **hash_htable,
            int hash_size)
{
    printf("htable of %d entries\n", hash_size);

    int i;
    for (i=0;i<hash_size;++i) {
        printf("Level %d\n", i);

        print_htable_entries(hash_htable[i]);
    }
}


void
print_htable_summary(struct hash_node **hash_htable,
                    int hash_size,
                    int count)
{
    printf("htable of %d entries\n", hash_size);

    int i;
    for (i=0;i<hash_size;++i) {
        struct hash_node *node = hash_htable[i];
        int level_count = 0;

        while (node) {
            ++level_count;
            node = node->next;
        }

        printf("Level %5d: %5d entries(%.2f%%)\n", i, level_count, ((float)level_count/(float)count)*100);
    }
}

int
contruct_htable(struct hash_data *data,
                int count,
                struct hash_node ***hash_htable,
                int hash_size,
                hash_function_t hash_function)
{
    int i;

    if (!(*hash_htable = malloc(hash_size*sizeof(struct hash_node *)))) {
        fprintf(stderr, "Error error allocating %d bytes: %s", hash_size*sizeof(struct hash_node *), strerror(errno));
        return -1;
    }

    for (i=0;i<hash_size;++i)
        (*hash_htable)[i] = NULL;

    for (i=0;i<count;++i) {
        struct hash_node *node = NULL;
        int hash = hash_function(data[i].key, hash_size-1);
        if (hash >= hash_size) { /* sanity checks */
            fprintf(stderr, "Invalid hash: %d for %s, skipping element\n", hash, data[i].key);
            continue;
        }

        if (!(node = malloc(sizeof(struct hash_node)))) {
            fprintf(stderr, "Error error allocating %d bytes: %s", sizeof(struct hash_node), strerror(errno));
            return -1;
        }

        node->data = &data[i];
        node->next = (*hash_htable)[hash];
        (*hash_htable)[hash] = node;
    }
}

void
destroy_htable_entries(struct hash_node *node)
{
    while (node) {
        struct hash_node *n = node;
        node = node->next;
        free(n);
    }
}

void
destroy_htable(struct hash_node **hash_htable,
                    int hash_size)
{
    int i;
    for (i=0;i<hash_size;++i)
        destroy_htable_entries(hash_htable[i]);

    free(hash_htable);
}

struct hash_node *
search_htable(struct hash_node **hash_htable,
              int hash_size,
              hash_function_t hash_function,
              const unsigned char *key)
{
    struct hash_node *result = NULL;
    struct hash_node *node;
    int hash = hash_function(key, hash_size-1);
    if (hash >= hash_size) { /* sanity checks */
        fprintf(stderr, "Invalid hash: %d for %s\n", hash, key);
        return NULL;
    }

    node = hash_htable[hash];

    while (node) {
        struct hash_node *needle;

        if (!key) {
            if (node->data->key)
                goto next;
        } else if (strcmp(key, node->data->key)) {
            goto next;
        }

        if (!(needle = malloc(sizeof(struct hash_node)))) {
            fprintf(stderr, "Error error allocating %d bytes: %s", sizeof(struct hash_node), strerror(errno));
            goto out;
        }
        needle->data = node->data;
        needle->next = result;
        result = needle;

      next:
        node = node->next;
    }

  out:
    return result;
}

int
simple_hash(const char *data,
            int limit)
{
    unsigned int hash = 0;
	int c;

    if (!data || limit == 0)
        return 0;

	while (c = *data++)
	    hash += c;

	return hash % limit;
}

/*
  dd if=/dev/urandom of=/tmp/rnd count=1000000 bs=32
  ./htable --hash-function simple --input-data /tmp/rnd
*/

int
usage(const char *prog)
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "%s --hash-function|-f simple --hash-size|-s size --key|-k key [--input-data|-i <path>] [--count|-c <num>] [--dump-data] [--dump-htable] [--dump-htable-summary]\n", prog);
    return 1;
}

int main(int argc, char **argv)
{
    struct timeval tb, ta;
    unsigned long secs, msecs, usecs;

    int dump_data = 0, dump_htable = 0, dump_htable_summary = 0;
    hash_function_t hash_function = NULL;
    int hash_size = 0;
    struct hash_node **hash_htable = NULL;
    int count = 0;
    const char *input_data = "/dev/random";
    struct hash_data *data;
    const unsigned char *key = NULL;
    struct hash_node *values;

    int opt;
    const struct option options[] = {
        {"hash-function", required_argument, NULL, 'f'},
        {"hash-size",     required_argument, NULL, 's'},
        {"key",           required_argument, NULL, 'k'},
        {"input-data",    required_argument, NULL, 'i'},
        {"count",         required_argument, NULL, 'c'},
        {"dump-htable",    no_argument,       &dump_htable, 1},
        {"dump-htable-summary",  no_argument, &dump_htable_summary, 1},
        {"dump-data",     no_argument,       &dump_data, 1},
        {0,               0,                 0,    0}
    };

    while ((opt = getopt_long(argc, argv, "f:s:k:i:c:", options, NULL)) != -1) {
        switch (opt) {
            case 'f':
                if (!strncmp(optarg, "simple", 6))
                    hash_function = simple_hash;
                else
                    return usage(argv[0]);
                break;
            case 'k':
                key = optarg;
                break;
            case 'c':
                count = atoi(optarg);
                break;
            case 's':
                hash_size = atoi(optarg);
                break;
            case 'i':
                input_data = optarg;
                break;
            case 0:
                break;
            default:
                return usage(argv[0]);
        }
    }
    if (hash_function == NULL || hash_size == 0)
        return usage(argv[0]);

    if (generate_data(&data, &count, input_data) == -1)
        return 1;
    printf("Elements: %d\n", count);
    if (dump_data)
        print_data(data, count);

    contruct_htable(data, count, &hash_htable, hash_size, hash_function);
    if (dump_htable)
        print_htable(hash_htable, hash_size);
    if (dump_htable_summary)
        print_htable_summary(hash_htable, hash_size, count);

    gettimeofday(&tb, NULL);
    values = search_htable(hash_htable, hash_size, hash_function, key);
    gettimeofday(&ta, NULL);

    if (values) {
        printf("Entries for key '%s'\n", key);
        print_htable_entries(values);
        destroy_htable_entries(values);
    } else {
        printf("Value was not found for key '%s'\n", key);
    }

    usecs = ta.tv_sec*1000000 + ta.tv_usec - tb.tv_sec*1000000 - tb.tv_usec;
    secs = usecs/1000000;
    usecs %= 1000000;
    msecs = usecs/1000;
    usecs %= 1000;
    printf("Search time: %lu seconds %lu msecs %lu usecs\n", secs, msecs, usecs);

  out:
    free(data);
    destroy_htable(hash_htable, hash_size);

    return 0;
}
