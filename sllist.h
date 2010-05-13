#ifndef _SLLIST_H
#define _SLLIST_H

struct sllist {
    struct sllist *next;
};

#define sllist_empty(head) ((head)->next == (head))

#define sllist_for_each(head, e)                                 \
    for ((e)=(head)->next;                                       \
         (e)!=(head);                                            \
         (e)=(e)->next)

#define sllist_for_each_prev(head, e, p)                         \
    for ((p)=(head), (e)=(head)->next;                           \
         (e)!=(head);                                            \
         (p)=(e), (e)=(e)->next)

#define sllist_for_each_safe(head, e, t)                         \
    for ((e)=(head)->next, (t)=(head)->next->next;               \
         (e)!=(head);                                            \
         (e)=(t), (t)=(t)->next)

#define sllist_for_each_safe_prev(head, e, p, t)                 \
    for ((p)=(head), (e)=(head)->next, (t)=(head)->next->next;   \
         (e)!=(head);                                            \
         (p)=((t)==(head)->next?(head):(e)), (e)=(t), (t)=(t)->next)

#define sllist_init(list)                                               \
    do {                                                                \
        (list)->next = (list);                                          \
    } while (0)

#define sllist_detach(e, prev)                                          \
    do {                                                                \
        (prev)->next = (e)->next;                                       \
        /* safe detach */                                               \
        (e)->next = (e);                                                \
    } while (0)

#define sllist_add(head, e)                                             \
    do {                                                                \
        (e)->next = (head)->next;                                       \
        (head)->next = (e);                                             \
    } while (0)

#endif
