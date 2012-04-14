#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <errno.h>
#include <getopt.h>

#include "helpers.h"

struct alpha_beta_gamma_filter_state {
    double alpha, beta, gamma;
    double dT;
    double pX;
    double pV;
    double pA;
};

void alpha_beta_gamma_filter_init(struct alpha_beta_gamma_filter_state *state)
{
}

double alpha_beta_gamma_filter(struct alpha_beta_gamma_filter_state *state, double in)
{
    double x;
    double r;

    x = state->pX + state->dT * state->pV + (state->dT * state->dT * state->pA)/2;
    r = in - x;
    state->pX += state->alpha * r;
    state->pV += (state->beta * r) / state->dT;
    state->pA += (state->gamma * r) / (state->dT * state->dT);

    return state->pX;
}

#ifdef ALPHA_BETA_GAMMA_MAIN
int
usage(const char *prog)
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "%s [--alpha|-a <beta>] [--beta|-b <beta>] [--gamma|-g <gamma>] [--delta-t|-t <delta T>]"
                    " [--input-data|-i <path>] [--count|-c <num>] [--output-data|-o <path>]\n", prog);
    return 1;
}

int main(int argc, char **argv)
{
    int ret = 1;
    const char *input_data = "/dev/random", *output_data = NULL;
    int count = 0;

    struct alpha_beta_gamma_filter_state abg_state;
    unsigned int *array_in, *array_out;

    int opt;
    const struct option options[] = {
        {"alpha",             required_argument, NULL, 'a'},
        {"beta",              required_argument, NULL, 'b'},
        {"gamma",             required_argument, NULL, 'g'},
        {"count",             required_argument, NULL, 'c'},
        {"delta-t",           required_argument, NULL, 't'},
        {"input-data",        required_argument, NULL, 'i'},
        {"output-data",       required_argument, NULL, 'o'},
        {0,                   0,                 0,    0}
    };

    memset(&abg_state, 0x0, sizeof(abg_state));

    while ((opt = getopt_long(argc, argv, "a:b:g:t:c:i:o:", options, NULL)) != -1) {
        switch (opt) {
            case 'a':
                abg_state.alpha = atof(optarg);
                break;
            case 'b':
                abg_state.beta = atof(optarg);
                break;
            case 'g':
                abg_state.gamma = atof(optarg);
                break;
            case 't':
                abg_state.dT = atof(optarg);
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

    printf("Alpha-Beta-Gamma parameters: alpha:%f beta:%f gamma:%f dT:%f\n",
            abg_state.alpha, abg_state.beta, abg_state.gamma, abg_state.dT);

    array_out = malloc(sizeof(unsigned int)*count);
    if (!array_out) {
        fprintf(stderr, "Unable to allocate %d bytes", count);
        goto out;
    }
    memset(array_out, 0x0, sizeof(unsigned int)*count);

    alpha_beta_gamma_filter_init(&abg_state);

    {
        int i;
        for (i=0;i<count;++i)
            array_out[i] = alpha_beta_gamma_filter(&abg_state, array_in[i]);
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
                fprintf(stderr, "Error error writing to %s: %s", output_data, strerror(errno));
                break;
            }
        }
        close(d);
    }

    ret = 0;
  out:
    if (array_in)
        free(array_in);
    if (array_out)
        free(array_out);

    return ret;
}
#endif
