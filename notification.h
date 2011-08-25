#ifndef _NOTIFICATION_H_
#define _NOTIFICATION_H_

struct notification {
    struct notification_desc *desc;
};

int init_notification(struct notification *notification);
void destroy_notification(struct notification *notification);
void wait_for_notification(struct notification *notification);
void notify(struct notification *notification);
void notify_all(struct notification *notification);

#endif
