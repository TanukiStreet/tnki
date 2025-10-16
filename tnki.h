/*
    ____  ____  ____  
   /\   \/\   \/\   \ 
  /  \___\ \___\ \___\
  \  /   /   ____    /
   \/___/\/ /\   \ _/ 
           /  \___\   
           \  /   /   
            \/___/    

    signed tanuki street 2025

    lgplv2.0 sucka

    da tnki library contains...
        - useful header macros
        - a generic stack implementation
        - an arena allocator
*/

#ifndef TNKI_H
#define TNKI_H

/* TODO: actually enforce this, currently just breaks */
#if !defined(TNKI_NO_STDC)
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#endif

/* ===::HELPER MACROS::=== */

#define Stmnt(S)                                                               \
    do                                                                         \
    {                                                                          \
        S                                                                      \
    } while (0)
#define Stringify_(S) #S
#define Stringify(S) Stringify_(S)
#define Glue_(A, B) A##B
#define Glue(A, B) Glue_(A, B)

#define ArrayCount(a) (sizeof(a) / sizeof(*(a)))

/* ===::DANGER ZONE::=== */

#define IntFromPtr(p) (unsigned long long)((char *)p - (char *)0)
#define PtrFromInt(n) (void *)((char *)0 + (n))

#define Member(T, m) (((T *)0)->m)
#define OffsetOfMember(T, m) IntFromPtr(&Member(T, m))
#define ContainerOf(ptr, type, member)                                         \
    ((type *)((char *)ptr - OffsetOfMember(type, member)))

#define Min(a, b) (((a) < (b)) ? (a) : (b))
#define Max(a, b) (((a) > (b)) ? (a) : (b))
#define Clamp(a, x, b) (((x) < (a)) ? (a) : ((b) < (x)) ? (b) : (x))
#define AbsoluteValue(a) (((a) < 0) ? -(a) : (a))

#define ClampTop(a, b) Min(a, b)
#define ClampBot(a, b) Max(a, b)

/* ===::DANGER ZONE 2.0::=== */

#if !defined(TNKI_NO_STDC)
#define MemoryCopy(d, s, z) memmove((d), (s), (z))
#define MemoryZero(p, z) memset((p), 0, (z))
#define MemoryMatch(a, b, z) (memcmp((a), (b), (z)) == 0)

#define MemoryZeroStruct(p) MemoryZero((p), sizeof(*(p)))
#define MemoryZeroArray(p) MemoryZero((p), sizeof(p))
#define MemoryZeroTyped(p, c) MemoryZero((p), sizeof(*(p)) * (c))

#define MemoryCopyStruct(d, s)                                                 \
    MemoryCopy((d), (s), Min(sizeof(*(d)), sizeof(*(s))))
#define MemoryCopyArray(d, s) MemoryCopy((d), (s), Min(sizeof(s), sizeof(d)))
#define MemoryCopyTyped(d, s, c)                                               \
    MemoryCopy((d), (s), Min(sizeof(*(d)), sizeof(*(s))) * (c))
#endif

/* ===:: STACK ::=== */

struct stk_node
{
    struct stk_node *next, *prev;
};

#define STKInitStatic(name) {&(name), &(name)}
#define STKInitDynamic(name)                                                   \
    Stmnt((name).next = &(name); (name).prev = &(name);)
#define STKContext(name) struct stk_node name = STKInitStatic(name)

static inline void
tnki_internal_stk_add(struct stk_node *entry,
                      struct stk_node *prev,
                      struct stk_node *next)
{
    next->prev = entry;
    entry->next = next;
    entry->prev = prev;
    prev->next = entry;
}

static inline void
tnki_internal_stk_del(struct stk_node *prev, struct stk_node *next)
{
    next->prev = prev;
    prev->next = next;
}

static inline bool
stk_empty(struct stk_node *context)
{
    return (context == context->next);
}

static inline struct stk_node *
stk_heads(struct stk_node *context)
{
    return context->next;
}

static inline struct stk_node *
stk_tails(struct stk_node *context)
{
    return context->prev;
}

static inline void
stk_push(struct stk_node *context, struct stk_node *entry)
{
    tnki_internal_stk_add(entry, context, context->next);
}

static inline struct stk_node *
stk_pop(struct stk_node *context)
{
    struct stk_node *top = context->next;
    tnki_internal_stk_del(context, context->next->next);
    return top;
}

static inline struct stk_node *
stk_peek(struct stk_node *context)
{
    return context->next;
}

#define stk_foreach(var, context)                                              \
    for ((var) = stk_peek(context); (var) != (context); (var) = (var)->next)

#define stk_foreach_safe(var, tvar, context)                                   \
    for ((var) = stk_peek(context);                                            \
         (var) != (context) && ((tvar) = (var)->next, 1);                      \
         (var) = (tvar))

/* ====::ALLOC OVERRIDE::==== */

#if !defined(TNKI_NO_STDC)
#ifndef TNKIMalloc
#define TNKIMalloc malloc
#endif

#ifndef TNKIRealloc
#define TNKIRealloc realloc
#endif

#ifndef TNKIFree
#define TNKIFree free
#endif
#endif

/* ===:: PAGES ::=== */

/*
 * an arena allocator
 *
 * like a notebook, filled with pages of
 * notes, this allocator creates a linked
 * list of contiguous memory blocks to be
 * filled with... whatever.
 *
 * NOTE:
 * run in debug mode first!! if an allocation
 * request is greater than a page size
 * yucky-ness can occur...
*/

#define Kb 1024
#define Mb (Kb*1024)

struct notebook;
struct page;

/* creates a linked list of pages
 * capacity - denotes page size
 * returns a new notebook
 */
struct notebook *
pages_create(size_t capacity);

/* resets the linked list of pages
 * notes - header for the page stack
 */
void
pages_reset(struct notebook *notes);

/* destroys list
 * notes - header for the page stack
 */
void
pages_destroy(struct notebook *notes);

/* allocate data in the notebook
 * notes - current notebook
 * size - size of data to allocate in bytes
 */
void *
pages_alloc(struct notebook *notes, size_t size);

/* reallocates data in the notebook
 * notes - current notebook
 * ptr - pointer to element that requires realloc
 * new_size - new size
 */
void *
pages_realloc(struct notebook *notes, void *ptr, size_t new_size);

/* does nothing */
static inline void
pages_free(void *ptr)
{}

#endif
