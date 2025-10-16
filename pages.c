#include "tnki.h"

/* linked list of pages */
struct notebook
{
    size_t capacity;
    struct stk_node pages;
};
/* a node dsecribing memory bounds */
struct page
{
    size_t size;
    uint8_t *data;
    const uint8_t *last_return; /* for optimization of realloc */
    struct stk_node page;
};
#define PageInfo(node) ContainerOf(node, struct page, page)

/* creates a linked list of pages
 * capacity - denotes page size
 * returns a new notebook
 */
struct notebook *
pages_create(size_t capacity)
{
    struct notebook *notes = TNKIMalloc(sizeof(struct notebook));
    assert(notes);
    STKInitDynamic(notes->pages);
    notes->capacity = capacity;
    return notes;
}

/* resets the linked list of pages
 * notes - header for the page stack
 */
void
pages_reset(struct notebook *notes)
{
    if (stk_empty(&notes->pages))
        return;
    struct page *first_page = PageInfo(stk_pop(&notes->pages));

    struct page *current_page;
    while (!stk_empty(&notes->pages))
    {
        current_page = PageInfo(stk_pop(&notes->pages));
        TNKIFree(current_page->data);
        TNKIFree(current_page);
    }

    first_page->size = 0;
    stk_push(&notes->pages, &first_page->page);
}

/* destroys list
 * notes - header for the page stack
 */
void
pages_destroy(struct notebook *notes)
{
    struct page *current_page;

    while (!stk_empty(&notes->pages))
    {
        current_page = PageInfo(stk_pop(&notes->pages));
        TNKIFree(current_page->data);
        TNKIFree(current_page);
    }

    TNKIFree(notes);
}

/* allocate data in the notebook
 * notes - current notebook
 * size - size of data to allocate in bytes
 */
void *
pages_alloc(struct notebook *notes, size_t size)
{
    struct page *new_page;

    assert(notes->capacity >= size);

    if (stk_empty(&notes->pages))
        goto NEW_PAGE;

    struct stk_node *current_node;
    stk_foreach(current_node, &notes->pages)
    {
        struct page *current_page = PageInfo(current_node);
        if (current_page->size + size > notes->capacity)
            continue;
        uint8_t *return_address = &current_page->data[current_page->size];
        current_page->last_return = return_address;
        current_page->size += size;
        return (void *)return_address;
    }

NEW_PAGE:
    new_page = TNKIMalloc(sizeof(struct page));
    assert(new_page);
    new_page->size = size;
    new_page->data = TNKIMalloc(sizeof(uint8_t) * notes->capacity);
    new_page->last_return = new_page->data;
    assert(new_page->data);
    stk_push(&notes->pages, &new_page->page);
    return (void *)new_page->data;
}

void *
pages_realloc(struct notebook *notes, void *ptr, size_t new_size)
{
    /* will attempt to return the same pointer if possible,
     * otherwise will iterate through the notebook until
     * room is found
     * best case:
     * |*^^^...|.....
     * return *
     * worst case:
     * |*^^^^^^|^^^|$...
     * return $ memcpy'd
     */
    struct page *ret_page;
    struct page *src_page;
    struct stk_node *spacious_node;
    uint8_t *memory_end_point;

    assert(notes->capacity >= new_size);
    assert(!stk_empty(&notes->pages));

    stk_foreach(spacious_node, &notes->pages)
    {
        ret_page = PageInfo(spacious_node);
        if (ret_page->last_return == ptr)
        {
            size_t old_size = ret_page->size - (ret_page->last_return - ret_page->data);
            if (ret_page->size - old_size + new_size <= notes->capacity)
            {
                ret_page->size += new_size - old_size;
                return (void*)ret_page->last_return;
            }
        }
        ret_page = PageInfo(spacious_node);
        if (ret_page->size + new_size > notes->capacity)
            continue;
        uint8_t *return_address = &ret_page->data[ret_page->size];
        ret_page->last_return = return_address;
        ret_page->size += new_size;
        goto MEMCPY;
    }

    ret_page = TNKIMalloc(sizeof(struct page));
    assert(ret_page);
    ret_page->size = new_size;
    ret_page->data = TNKIMalloc(sizeof(uint8_t) * notes->capacity);
    ret_page->last_return = ret_page->data;
    assert(ret_page->data);
    stk_push(&notes->pages, &ret_page->page);

MEMCPY:
    /* find the page containing the ptr */
    memory_end_point = NULL;
    stk_foreach(spacious_node, &notes->pages)
    {
        src_page = PageInfo(spacious_node);
        memory_end_point = src_page->data + notes->capacity;
        if ((uint8_t*)ptr < memory_end_point)
            break;
        /* an invalid pointer was passed */
        assert(false);
    }
    /* copy up to new size or memory end point */
    if (!memory_end_point)
        return NULL;
    if (((uint8_t*)ptr + new_size) > memory_end_point)
        MemoryCopy((void*)ret_page->last_return, ptr, (size_t)(memory_end_point - (uint8_t*)ptr));
    else
        MemoryCopy((void*)ret_page->last_return, ptr, new_size);
    return (void *)ret_page->last_return;
}
