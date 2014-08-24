#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/time.h>
#include <pthread.h>
#include <errno.h>
#include <getopt.h>

#include "helpers.h"
#include "isort.h"
#include "dllist.h"
#include "tsort.h"

struct tsort_run {
    struct dllist list;
    unsigned int start;
    unsigned int size;
};

static unsigned int
get_minrun(unsigned int n)
{
    unsigned int r = 0;

    while (n >= 64) {
        r |= n & 1;
        n >>= 1;
    }

    return n + r;
}

void tim_sort(void *array,
        unsigned int count,
        unsigned int size,
        assign_t assign,
        swap_t swp,
        compare_t cmp)
{
    struct dllist runL;
    unsigned int runN;
    struct tsort_run *run;
    unsigned int mrun, crun;
    unsigned int curr, rstart, rsize;
    bool sort;

    if (count <= 1)
        return;

    if (2 == count) {
        if (cmp(array, array+size) == cmp_result_greater)
            swp(array, array+size);

        return;
    }

    mrun = get_minrun(count);
    dllist_init(&runL);

    runN = 0;
    curr = 0;
    while (curr < count) {
        crun = min(mrun, count-curr);
        rsize = 1;
        rstart = curr;
        sort = false;
        for (;;) {
            if (cmp(array+curr*size, array+(curr+1)*size) == cmp_result_greater) {
                if (rsize >= crun)
                    break;
                swp(array+curr*size, array+(curr+1)*size);
                sort = true;
            }

            ++curr;
            ++rsize;
        }
        if (sort)
            insertion_sort(array+rstart*size, rsize, size, assign, swp, cmp);

        ++curr;

        if (curr == count && 0 == runN)
            return;

        if (!(run = (struct tsort_run *)malloc(sizeof(struct tsort_run)))) {
            fprintf(stderr, "Error allocating %d bytes: %s", sizeof(struct tsort_run), strerror(errno));
            goto err;
        }

        run->start = rstart;
        run->size = rsize;

        dllist_add_tail(&runL, &run->list);
        ++runN;
    }

    if (runN%2) {
        ALGO_ASSERT("tsort", runN%2, "");
    }

    while (!dllist_empy(&runL)) {
        struct tsort_run *run0, *run1;
        unsigned int c0, c1, cA;
        void *t;

        run0 = container_of(dllist_first(&runL), struct tsort_run, list);
        dllist_detach(&run0->list);
        run1 = container_of(dllist_first(&runL), struct tsort_run, list);
        dllist_detach(&run1->list);

        /* printf("run0<%u %u>, run1<%u %u>\n", */
        /*         run0->start, run0->size, */
        /*         run1?run1->start:-1, run1?run1->size:0); */

        if (!(t = malloc(run0->size*size))) {
            fprintf(stderr, "Error allocating %d bytes: %s", run0->size*size, strerror(errno));
            goto err;
        }

        memcpy(t, array+run0->start*size, run0->size*size);

        c0 = c1 = 0;
        cA = run0->start;
        while (c0 < run0->size || c1 < run1->size) {
            /* printf("c0 %u, c1 %u, cA %u\n", */
            /*         c0, c1, cA); */
            if ((c0 < run0->size && cmp(t+c0*size, array+(run1->start+c1)*size) == cmp_result_less)
                    || c1 >= run1->size) {
                ALGO_ASSERT("tsort", c0 < run0->size, "");
                assign(array+cA*size, t+c0*size);
                ++c0;
            } else {
                ALGO_ASSERT("tsort", c1 < run1->size, "");
                assign(array+cA*size, array+(run1->start+c1)*size);
                ++c1;
            }

            ALGO_ASSERT("tsort", cA < (run0->size+run1->size), "");

            ++cA;
        }

        free(run0);
        free(run1);
    }

    return;

  err:
    while (!dllist_empy(&runL)) {
        run = container_of(dllist_first(&runL), struct tsort_run, list);
        dllist_detach(&run->list);
        free(run);
    }
}
