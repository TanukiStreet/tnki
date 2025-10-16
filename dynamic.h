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

    a template for dynamic arrays

    unfinished implementation of a dynamic array
    based off of the article below...
    https://www.davidpriver.com/ctemplates.html
*/

/* ===:: header stub ::=== */
#ifndef DYNAMIC_H
#define DYNAMIC_H

#include "tnki.h"

/* combines a prefix with function name to generate a unique
 * function declaration.
 *
 * eg: dynamic_int_push(...), dynamic_float_push(...)
 */
#define dynamic_impl(word) Glue(dynamic_prefix,word)

#endif

/* ===:: template ::=== */

/* the type of the dyanmic template. mandatory!
 *
 * eg: dynamic<int>, dynamic<float>
 */
#ifndef dynamic_type
#error "dynamic_type must be defined"
#endif

/* name of the struct that will be created, this
 * is overridable.
 */
#ifndef dynamic_struct
#define dynamic_struct Glue(dynamic_,dynamic_type)
#endif

/* name of the prefix used for unique function declarations,
 * this is overridable.
 */
#ifndef dynamic_prefix
#define dynamic_prefix Glue(dynamic_,dynamic_type)
#endif

/* the link type of the function, again overridable but
 * the default is static inline so that the functions
 * are created in the same module this file is included in.
 */
#ifndef dynamic_link
#define dynamic_link static inline
#endif

/* ===:: template implementation ::=== */
#define dynamic_push dynamic_impl(push)

struct dynamic_struct {
    dynamic_type *items;
    size_t count;
    size_t capacity;
};

/* if the template just ain't it chief, or if definitions
 * are wanted in a separate compilation unit.
 */
#ifdef dynamic_decls_only

dynamic_link
void
dynamic_push(dynamic_struct *array, dynamic_type item);

#undef dynamic_decls_only
#else

dynamic_link
void
dynamic_push(dynamic_struct *array, dynamic_type item) {
    if (array->count >= array->capacity) {
        size_t old_cap = array->capacity;
        size_t new_cap = old_cap ? old_cap * 2 : 4;
        size_t new_size = new_cap * sizeof(dynamic_type);
        array->items = (int*)TNKIRealloc(array->items, new_size);
        array->capacity = new_cap;
    }
    array->items[array->count++] = item;
}

#endif

/* macro fluff must be cleaned up for reuse in later
 * includes.
 */
#undef dynamic_push
#undef dynamic_link
#undef dynamic_prefix
#undef dynamic_struct
#undef dynamic_type
