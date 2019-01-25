#include<test.h>
#include<stdio.h>

#define HEAP_SIZE (4 * 1024 * 1024)

/* the heap has a size of 4 MiB. */

/* Your task is to implment a dynamic memory allocator */

/* As a simplification, you may assume that all allocations will be */
/* of the size 16 bytes or smaller */

/* As a debugging aid, printf() may be used */

/* Other than printf(), you may not use any C standard library function or any other code */

/* If you need any auxiliary memory storage for your work,*/
/* then that memory storage shall be from this heap buffer only */
/* NOTE that this may reduce the effective heap size from the application's point of view */

static uint8_t heap_buffer[HEAP_SIZE];

/* this file MUST NOT expose any symbols other than  */
/* custom_free AND custom_malloc */

/* You may not use any other dynamic memory allocator in this file */

/* You may not use any memory outside the heap_buffer */

void * custom_malloc(const int32_t n)
    {
    /* FINISH THIS */
    /* You may return a null pointer if n > MAX_ALLOC */
    return NULL;
    }

void custom_free(const void * ptr)
    {
    /* FINISH THIS */
    return;
    }

/* Compiling this file as it is gives some unused variable errors. */

/* Your final code must not give any warnings */

/* You shall not use C++ for this task, */

/* This task must be done using C language only. */
