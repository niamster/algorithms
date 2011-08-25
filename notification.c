#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "notification.h"

struct notification_desc {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
};

int init_notification(struct notification *notification)
{
    notification->desc = malloc(sizeof(struct notification_desc));
    if (!notification->desc) {
        printf("Unable to allocate notification descriptor\n");
        return -1;
    }

    if (pthread_mutex_init(&notification->desc->mutex, NULL) != 0) {
        printf("Unable to init notification lock\n");
        free(notification->desc);
        return -1;
    }

    if (pthread_cond_init(&notification->desc->cond, NULL) != 0) {
        printf("Unable to init notification notificator\n");
        free(notification->desc);
        return -1;
    }

    return 0;
}

void destroy_notification(struct notification *notification)
{
    pthread_mutex_destroy(&notification->desc->mutex);
    pthread_cond_destroy(&notification->desc->cond);
    free(notification->desc);
}

void wait_for_notification(struct notification *notification)
{
    pthread_mutex_lock(&notification->desc->mutex);
    if (pthread_cond_wait(&notification->desc->cond, &notification->desc->mutex) != 0) {
        printf("Failed to wait for event\n");
        return;
    }
    pthread_mutex_unlock(&notification->desc->mutex);
}

void notify(struct notification *notification)
{
    pthread_cond_signal(&notification->desc->cond);
}

void notify_all(struct notification *notification)
{
    pthread_cond_broadcast(&notification->desc->cond);
}
