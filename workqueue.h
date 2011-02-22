#ifndef _WORKQUEUE_H_
#define _WORKQUEUE_H_

#include "sllist.h"

typedef void (*work_fn_t)(void *);
typedef void (*notification_fn_t)(void *);

enum workState {
    WORK_RUNNING,
    WORK_PENDING,
    WORK_FINISHED,
    WORK_PURGED,
};

struct work {
    struct sllist list;

    work_fn_t fn;
    void *data;

    notification_fn_t notification_fn;
    void *notification_data;

    unsigned long state;
};

struct workqueue_desc;

struct workqueue {
    struct sllist works;
    struct workqueue_desc *desc;
};

int init_workqueue(struct workqueue *wq, unsigned int threads);
void flush_workqueue(struct workqueue *wq);
void destroy_workqueue(struct workqueue *wq);

void add_work(struct workqueue *wq, struct work *work);

#endif
