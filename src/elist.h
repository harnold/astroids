#ifndef ELIST_H
#define ELIST_H

#include <stdbool.h>

struct elist_node {
    struct elist_node *next;
    struct elist_node *prev;
};

struct elist {
    struct elist_node end;
};

static inline void init_elist(struct elist *list);
static inline struct elist_node *elist_begin(struct elist *list);
static inline struct elist_node *elist_end(struct elist *list);
static inline bool elist_empty(struct elist *list);
static inline void elist_insert(struct elist_node *node, struct elist_node *next);
static inline void elist_insert_front(struct elist_node *node, struct elist *list);
static inline void elist_insert_back(struct elist_node *node, struct elist *list);
static inline void elist_remove(struct elist_node *node);
static inline void elist_move(struct elist_node *node, struct elist_node *next);
static inline void elist_splice(struct elist_node *begin, struct elist_node *end,
                                struct elist_node *next);

#define elist_get(__node, __Type, __Member) \
    ((__Type *) (((char *) (__node)) - offsetof(__Type, __Member)))

#define elist_for_each_node(__node, __list) \
    for ((__node) = elist_begin(__list); \
         (__node) != elist_end(__list); \
         (__node) = (__node)->next)

#define elist_for_each_node_reverse(__node, __list) \
    for ((__node) = elist_end(__list)->prev; \
         (__node) != elist_end(__list); \
         (__node) = (__node)->prev)

#define elist_for_each_node_safe(__node, __tmp, __list) \
    for ((__node) = elist_begin(__list), (__tmp) = (__node)->next; \
         (__node) != elist_end(__list); \
         (__node) = (__tmp), (__tmp) = (__node)->next)

#define elist_for_each_elem(__elem, __list, __Type, __Member) \
    for ((__elem) = elist_get(elist_begin(__list), __Type, __Member); \
         &(__elem)->__Member != elist_end(__list); \
         (__elem) = elist_get((__elem)->__Member.next, __Type, __Member))

#define elist_for_each_elem_reverse(__elem, __list, __Type, __Member) \
    for ((__elem) = elist_get(elist_end(__list)->prev, __Type, __Member); \
         &(__elem)->__Member != elist_end(__list); \
         (__elem) = elist_get((__elem)->__Member.prev, __Type, __Member))

static inline void elist_link(struct elist_node *prev, struct elist_node *next)
{
    prev->next = next;
    next->prev = prev;
}

static inline void init_elist(struct elist *list)
{
    list->end.next = &list->end;
    list->end.prev = &list->end;
}

static inline struct elist_node *elist_begin(struct elist *list)
{
    return list->end.next;
}

static inline struct elist_node *elist_end(struct elist *list)
{
    return &list->end;
}

static inline bool elist_empty(struct elist *list)
{
    return elist_begin(list) == elist_end(list);
}

static inline void elist_insert(struct elist_node *node, struct elist_node *next)
{
    elist_link(next->prev, node);
    elist_link(node, next);
}

static inline void elist_insert_front(struct elist_node *node, struct elist *list)
{
    elist_insert(node, elist_begin(list));
}

static inline void elist_insert_back(struct elist_node *node, struct elist *list)
{
    elist_insert(node, elist_end(list));
}

static inline void elist_remove(struct elist_node *node)
{
    elist_link(node->prev, node->next);
    node->next = node;
    node->prev = node;
}

static inline void elist_move(struct elist_node *node, struct elist_node *next)
{
    elist_link(node->prev, node->next);
    elist_insert(node, next);
}

void elist_splice(struct elist_node *begin, struct elist_node *end,
                  struct elist_node *next)
{
    if (begin == end)
        return;

    struct elist_node *next_prev = next->prev;

    elist_link(end->prev, next);
    elist_link(begin->prev, end);
    elist_link(next_prev, begin);
}

#endif
