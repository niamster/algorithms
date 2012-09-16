#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <errno.h>
#include <getopt.h>

#include "helpers.h"

struct moving_average_filter_state {
    int idx;
    int P;
    double *b;
    double *samples;
};

int moving_average_filter_init(struct moving_average_filter_state *state)
{
    state->idx = 0;

    state->b = malloc(state->P * sizeof(double));
    if (!state->b) {
        fprintf(stderr, "Error allocating %d bytes: %s", state->P * sizeof(double), strerror(errno));
        return -1;
    }

    {
        int i;
        double b = 1./((double)state->P + 1);
        for (i=0;i<state->P;++i)
            state->b[i] = b;
    }

    state->samples = calloc(state->P, sizeof(double));
    if (!state->samples) {
        fprintf(stderr, "Error allocating %d bytes: %s", state->P * sizeof(double), strerror(errno));
        free(state->b);
        return -1;
    }

    return 0;
}

void moving_average_filter_destroy(struct moving_average_filter_state *state)
{
    if (state->b)
        free(state->b);
    if (state->samples)
        free(state->samples);
}

double moving_average_filter(struct moving_average_filter_state *state, double in)
{
    double acc = 0;
    int i;

    state->samples[state->idx] = in;

    for (i=0;i<state->P;++i)
        acc += state->samples[i]*state->b[i];

    ++state->idx;
    state->idx %= state->P;

    return acc;
}

#ifdef MOVING_AVERAGE_MAIN
int
usage(const char *prog)
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "%s [--order|-P <order>] [--input-data|-i <path>] [--count|-c <num>] [--output-data|-o <path>]\n", prog);
    return 1;
}

int main(int argc, char **argv)
{
    int ret = 1;

    const char *input_data = "/dev/random", *output_data = NULL;
    int count = 0;

    struct moving_average_filter_state ma_state;
    unsigned int *array_in, *array_out;

    int opt;
    const struct option options[] = {
        {"order",             required_argument, NULL, 'P'},
        {"count",             required_argument, NULL, 'c'},
        {"input-data",        required_argument, NULL, 'i'},
        {"output-data",       required_argument, NULL, 'o'},
        {0,                   0,                 0,    0}
    };

    memset(&ma_state, 0x0, sizeof(ma_state));

    while ((opt = getopt_long(argc, argv, "P:c:i:o:", options, NULL)) != -1) {
        switch (opt) {
            case 'P':
                ma_state.P = atof(optarg);
                break;
            case 'c':
                count = atoi(optarg);
                break;
            case 'i':
                input_data = optarg;
                break;
            case 'o':
                output_data = optarg;
                break;
            case 0:
                break;
            default:
                return usage(argv[0]);
        }
    }

    if (generate_array(&array_in, &count, input_data) == -1)
        return 1;

    printf("Elements: %d\n", count);
    {
        int i;
        for (i=0;i<count;++i)
            array_in[i] %= 100;
    }

    if (moving_average_filter_init(&ma_state))
        goto out;

    printf("Moving Average parameters: P:%d\n",
            ma_state.P);
    {
        int i;

        printf("b:\n");
        for (i=0;i<ma_state.P;++i)
            printf("%f ", ma_state.b[i]);
        printf("\n");
    }

    array_out = malloc(sizeof(unsigned int)*count);
    if (!array_out) {
        fprintf(stderr, "Unable to allocate %d bytes", count);
        goto out;
    }
    memset(array_out, 0x0, sizeof(unsigned int)*count);

    {
        int i;

        for (i=0;i<count;++i)
            array_out[i] = moving_average_filter(&ma_state, array_in[i]);
    }

    if (output_data) {
        int i;
        int d = open(output_data, O_WRONLY|O_CREAT, 0666);
        if (d == -1) {
            fprintf(stderr, "Error opening %s: %s", output_data, strerror(errno));
            goto out;
        }
        ftruncate(d, 0);
        for (i=0;i<count;++i) {
            char b[1024] = {0x0, };
            int n;
            n = snprintf(b, sizeof(b), "%d %d %d\n", i, array_in[i], array_out[i]);
            if (write(d, b, n) == -1) {
                fprintf(stderr, "Error writing to %s: %s", output_data, strerror(errno));
                break;
            }
        }
        close(d);
    }

    ret = 0;
  out:
    moving_average_filter_destroy(&ma_state);
    if (array_in)
        free(array_in);
    if (array_out)
        free(array_out);

    return ret;
}
#endif
