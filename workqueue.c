#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <getopt.h>

#include "sllist.h"
#include "helpers.h"
#include "notification.h"
#include "workqueue.h"

struct workqueue_desc {
    pthread_mutex_t mutex;
    struct notification notification;
    unsigned int num;
    unsigned int stopping:1;
    pthread_t threads[0];
};

static void *
worker(struct workqueue *wq)
{
    struct sllist *e, *p, *t;
    struct work *w;

    for (;;) {
        pthread_mutex_lock(&wq->desc->mutex);

        if (wq->desc->stopping) {
            pthread_mutex_unlock(&wq->desc->mutex);
            return NULL;
        }

        pthread_mutex_unlock(&wq->desc->mutex);

        wait_for_notification(&wq->desc->notification);

        pthread_mutex_lock(&wq->desc->mutex);

        if (wq->desc->stopping) {
            pthread_mutex_unlock(&wq->desc->mutex);
            return NULL;
        }

      retry:
        sllist_for_each_safe_prev(&wq->works, e, p, t) {
            w = container_of(e, struct work, list);
            sllist_detach(e, p);

            pthread_mutex_unlock(&wq->desc->mutex);

            w->state = WORK_RUNNING;
            w->fn(w->data);
            w->state = WORK_FINISHED;
            if (w->notification_fn)
                w->notification_fn(w->notification_data);

            pthread_mutex_lock(&wq->desc->mutex);
            goto retry;
        }

        pthread_mutex_unlock(&wq->desc->mutex);
    }

    return NULL;
}

static void
wake_up_all_workers(struct workqueue *wq)
{
    notify_all(&wq->desc->notification);
}

static void
wake_up_one_worker(struct workqueue *wq)
{
    notify(&wq->desc->notification);
}

int
init_workqueue(struct workqueue *wq, unsigned int threads)
{
    wq->desc = malloc(sizeof(struct workqueue_desc) + threads*sizeof(pthread_t));
    if (!wq->desc) {
        printf("Unable to allocate wq descriptor\n");
        return -1;
    }

    if (init_notification(&wq->desc->notification) != 0) {
        printf("Unable to init notification\n");
        free(wq->desc);
        return -1;
    }

    if (pthread_mutex_init(&wq->desc->mutex, NULL) != 0) {
        printf("Unable to init wq lock\n");
        free(wq->desc);
        return -1;
    }

    wq->desc->stopping = 0;

    for (wq->desc->num=0; wq->desc->num<threads;++wq->desc->num) {
        if (pthread_create(&wq->desc->threads[wq->desc->num], NULL, (void *(*)(void*))worker, wq) != 0) {
            printf("Unable to create wq thread\n");

            wq->desc->stopping = 1;

            wake_up_all_workers(wq);

            for (;wq->desc->num!=0;--wq->desc->num) {
                pthread_join(wq->desc->threads[wq->desc->num-1], (void **)NULL);
            }

            free(wq->desc);
            return -1;
        }
    }

    sllist_init(&wq->works);

    return 0;
}

void
flush_workqueue(struct workqueue *wq)
{
    for (;;) {
        pthread_mutex_lock(&wq->desc->mutex);
        if (sllist_empty(&wq->works)) {
            pthread_mutex_unlock(&wq->desc->mutex);
            return;
        }
        pthread_mutex_unlock(&wq->desc->mutex);

        wake_up_all_workers(wq);
    }
}

void
destroy_workqueue(struct workqueue *wq)
{
    struct sllist *e, *p, *t;
    struct work *w;

    pthread_mutex_lock(&wq->desc->mutex);
    wq->desc->stopping = 1;

    sllist_for_each_safe_prev(&wq->works, e, p, t) {
        w = container_of(e, struct work, list);

        w->state = WORK_PURGED;
        if (w->notification_fn)
            w->notification_fn(w->notification_data);

        sllist_detach(e, p);
    }
    pthread_mutex_unlock(&wq->desc->mutex);

    wake_up_all_workers(wq);

    for (;wq->desc->num!=0;--wq->desc->num) {
        pthread_join(wq->desc->threads[wq->desc->num-1], (void **)NULL);
    }

    pthread_mutex_destroy(&wq->desc->mutex);
    destroy_notification(&wq->desc->notification);
    free(wq->desc);
}

void
add_work(struct workqueue *wq, struct work *work)
{
    work->state = WORK_RUNNING;

    pthread_mutex_lock(&wq->desc->mutex);
    sllist_add(&wq->works, &work->list);
    pthread_mutex_unlock(&wq->desc->mutex);

    wake_up_one_worker(wq);
}

#ifdef WORKQUEUE_MAIN
void __worker(void *data)
{
    printf("work %z\n", (size_t)data);
}

void __notification(void *data)
{
    printf("finished %u\n", (size_t)data);
}

int
usage(const char *prog)
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "%s [--threads|-t <num>] [--works|-w <num>]\n", prog);
    return 1;
}

int main(unsigned int argc, char **argv)
{
    struct timeval tb, ta;
    unsigned long secs, msecs, usecs;

    unsigned long works = 0;
    unsigned int threads = 0;

    struct workqueue wq;
    struct work *ws;

    int opt;
    const struct option options[] = {
        {"threads",       required_argument, NULL, 't'},
        {"works",         required_argument, NULL, 'w'},
        {0,               0,                 0,    0}
    };

    while ((opt = getopt_long(argc, argv, "t:w:", options, NULL)) != -1) {
        switch (opt) {
            case 't':
                threads = atoi(optarg);
                break;
            case 'w':
                works = atoi(optarg);
                break;
            case 0:
                break;
            default:
                return usage(argv[0]);
        }
    }

    ws = malloc(sizeof(struct work)*works);
    if (!ws) {
        printf("Unable to allocate %u bytes for works\n", sizeof(struct work)*works);
        return 1;
    }

    if (init_workqueue(&wq, threads) != 0) {
        printf("Unable to init workqueue\n");
        free(ws);
        return 1;
    }

    for (;works!=0;--works) {
        struct work *w = &ws[works-1];

        w->fn = __worker;
        w->data = (void *)works;

        w->notification_fn = __notification;
        w->notification_data = (void *)works;

        add_work(&wq, w);
    }

    flush_workqueue(&wq);
    destroy_workqueue(&wq);

    free(ws);

    return 0;
}
#endif
