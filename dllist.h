#ifndef _DLLIST_H
#define _DLLIST_H

struct dllist {
    struct dllist *next;
    struct dllist *prev;
};

static inline void
dllist_init(struct dllist *head)
{
    head->next = head->prev = head;
}

static inline bool
dllist_empy(struct dllist *head)
{
    return head->next == head && head->prev == head;
}

static inline struct dllist *
dllist_first(struct dllist *head)
{
    return head->next;
}

static inline struct dllist *
dllist_last(struct dllist *head)
{
    return head->prev;
}

#define dllist_for_each(head, e)                \
    for ((e)=(head)->next;                      \
         (e)!=(head);                           \
         (e)=(e)->next)

#define dllist_for_each_safe(head, e, t)        \
    for ((t)=(head)->next->next, (e)=(t)->prev; \
         (e)!=(head);                           \
         (t)=(t)->next, (e)=(t)->prev)

static inline void
dllist_detach(struct dllist *e)
{
    e->prev->next = e->next;
    e->next->prev = e->prev;
}

static inline void
dllist_add(struct dllist *head, struct dllist *e)
{
    head->next->prev = e;
    e->next = head->next;
    e->prev = head;
    head->next = e;
}

static inline void
dllist_add_tail(struct dllist *head, struct dllist *e)
{
    head->prev->next = e;
    e->prev = head->prev;
    head->prev = e;
    e->next = head;
}

#endif
