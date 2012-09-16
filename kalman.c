#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <errno.h>
#include <getopt.h>

#include "helpers.h"

struct kalman_filter_state {
    double F, B, H;
    double Q, R;
    double pValue;
    double pControl;
    double pCovError;
    double K;
    double I;
};

void kalman_filter_init(struct kalman_filter_state *state, double covError, double value)
{
    state->pCovError = covError;
    state->pValue = value;
}

double kalman_filter(struct kalman_filter_state *state, double in)
{
    double cValue;
    double cCovError;

    // prediction
    cValue = state->F*state->pValue + state->B*state->pControl;
    cCovError = state->F*state->pCovError*state->F + state->Q;

    // update
    state->K = (cCovError*state->H)/(state->H*cCovError*state->H + state->R);
    state->pValue = cValue + state->K*(in - state->H*cValue);
    state->pCovError = (state->I - state->K*state->H)*cCovError;

    return state->pValue;
}

#ifdef KALMAN_MAIN
int
usage(const char *prog)
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "%s [--control-input|-B <control input>] [--state-transition|-F <state transition>]"
                    " [--observation|-H <observation>] [--process-noise-cov|-Q <process noise covariation>]"
                    " [--meas-noise-cov|-R <measurement noise covariation>]"
                    " [--input-data|-i <path>] [--count|-c <num>] [--output-data|-o <path>]\n", prog);
    return 1;
}

int main(int argc, char **argv)
{
    int ret = 1;
    const char *input_data = "/dev/random", *output_data = NULL;
    int count = 0;

    struct kalman_filter_state k_state;
    unsigned int *array_in, *array_out;

    int opt;
    const struct option options[] = {
        {"control-input",     required_argument, NULL, 'B'},
        {"state-transition",  required_argument, NULL, 'F'},
        {"observation",       required_argument, NULL, 'H'},
        {"process-noise-cov", required_argument, NULL, 'Q'},
        {"meas-noise-cov",    required_argument, NULL, 'R'},
        {"count",             required_argument, NULL, 'c'},
        {"input-data",        required_argument, NULL, 'i'},
        {"output-data",       required_argument, NULL, 'o'},
        {0,                   0,                 0,    0}
    };

    memset(&k_state, 0x0, sizeof(k_state));
    k_state.I = 1;

    while ((opt = getopt_long(argc, argv, "B:F:H:Q:R:c:i:o:", options, NULL)) != -1) {
        switch (opt) {
            case 'B':
                k_state.B = atof(optarg);
                break;
            case 'F':
                k_state.F = atof(optarg);
                break;
            case 'H':
                k_state.H = atof(optarg);
                break;
            case 'Q':
                k_state.Q = atof(optarg);
                break;
            case 'R':
                k_state.R = atof(optarg);
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

    printf("Kalman parameters: Q:%f R:%f I:%f F:%f H:%f B:%f\n",
            k_state.Q, k_state.R, k_state.I, k_state.F, k_state.H, k_state.B);

    array_out = malloc(sizeof(unsigned int)*count);
    if (!array_out) {
        fprintf(stderr, "Unable to allocate %d bytes", count);
        goto out;
    }
    memset(array_out, 0x0, sizeof(unsigned int)*count);

    {
        int i;
        kalman_filter_init(&k_state, 0.1, array_in[0]);
        for (i=1;i<count;++i)
            array_out[i] = kalman_filter(&k_state, array_in[i]);
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
    if (array_in)
        free(array_in);
    if (array_out)
        free(array_out);

    return ret;
}
#endif
