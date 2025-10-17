# TNKI C
a simple codebase for simple needs.

includes...
- useful header macros
- a generic stack implementation
- an arena allocator

## documentation
useful macros and predefs are in tnki.h. each block is divided by *===:: named ::===* sections. read the comments for the general gist. 

### helper macros
stuff that should be prebuilt in c... but ain't.

### danger zone
fun pointer arithmetic tricks, noteably *ContainerOf* and *OffsetOfMember* are useful for getting data out of opaque structs. this stuff can be gnarly though so use with caution.

### stack
a stack implementation; a mix between openbsd's queue and the linux kernel's linked list.

refer to pages.c for an example of usage.

*STKContext* creates a struct stk_node and fills it's pointers with reference to itself. useful for declaring on the stack (memory stack / opposite of heap). *STKInitStatic* is just a helper macro for *STKContext*.

### currently incomplete
strcon.c has some string conversion algorithms.

dynamic.h is a generic header that creates a dynamic array of whatever type you need.
