#ifndef _LIST_H_
#define _LIST_H_

#include "rtos.h"


#ifdef __cplusplus
extern "C" {
#endif

struct list
{
    struct list *prev, *next;
};
typedef struct list list_t;

/*  */
#define CONTAIN_OF(type, member) (&(((type *)0)->member))

/*  */
#define LIST_ENTRY(pos, type, member) ((type *)( (char *)pos - (unsigned long)CONTAIN_OF(type, member)))

/*  */
#define LIST_HEAD_ENTRY(head, type, member) LIST_ENTRY((head)->next, type, member)

/*  */
#define LIST_TAIL_ENTRY(head, type, member) LIST_ENTRY((head)->prev, type, member)

/*  */
#define LIST_NEXT_ENTRY(elm, type, member)  LIST_ENTRY((elm)->member.next, type, member)

/*  */
#define LIST_FOR_EACH_ENTRY( pos, list, type, member) \
    for( (pos) = LIST_HEAD_ENTRY(list, type, member); \
         &(pos)->member != (list); \
         (pos) = LIST_NEXT_ENTRY(pos, type, member))
      
#define LIST_FOR_EACH_ENTRY_SAFE( pos, n, list, type, member) \
    for( (pos) = LIST_HEAD_ENTRY(list, type, member), \
         (n) = LIST_NEXT_ENTRY(pos, type, member); \
         &(pos)->member != (list); \
         (pos) = (n), (n) = LIST_NEXT_ENTRY(pos, type, member))

#define LIST_FOR_EACH_ENTRY_FROM_PTR(pos, head, tail, type, member) \
    for( (pos) = (head); \
         (pos) != (tail); \
         (pos) = LIST_NEXT_ENTRY(pos, type, member))
      
/*  */
#define LIST_FOR_EACH_HEAD_NEXT(pos, list, type, member) \
    for( (pos) = LIST_HEAD_ENTRY(list, type, member); \
         &(pos)->member != (list); \
         (pos) = LIST_HEAD_ENTRY(list, type, member))

/*  */
INLINE void list_init(list_t *link)
{
    link->prev = link;
    link->next = link;
}

/*  */
INLINE int list_is_empty(list_t *link)
{
    return (link->next == link);
}

/*  */
INLINE void __list_insert(list_t *prev,
                          list_t *next,
                          list_t *node)
{
    next->prev = node;
    node->next = next;
    node->prev = prev;
    prev->next = node;     
}

#define list_insert_head(link, node) __list_insert(link, (link)->next, node)
#define list_insert_tail(link, node) __list_insert((link)->prev, link, node)

/*  */
INLINE void list_remove_node(list_t *node)
{
    node->prev->next = node->next;
    node->next->prev = node->prev;
}


/******************************************************************************/
    
#ifdef __cplusplus
}
#endif

#endif
