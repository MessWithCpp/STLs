#ifndef TEST_H
#define TEST_H

#include<stdint.h>

#define MAX_ALLOC (16)

/* this function implements a dynamic memory allocator */

/* this function returns null pointer if heap memory is exhausted */
/* or if n > MAX_ALLOC */
void * custom_malloc(const int32_t n);
#define HEAP_SIZE (4 * 1024 * 1024)
struct malloc_chunk_header
{
	int size;
	struct malloc_chunk_header *next;
};

static struct malloc_chunk_header *alloca_head;
static struct malloc_chunk_header *alloca_tail;

static struct malloc_chunk_header *free_head;
static struct malloc_chunk_header *free_tail;

static int remaining_heap=HEAP_SIZE;

static uint8_t heap_buffer[HEAP_SIZE];

static uint8_t *heap_top = heap_buffer;

/* this function frees memory allocated by custom_malloc */

/* This function must not do anything if it is asked to deallocate memory */
/* which was not allocated by custom_malloc() */
void custom_free(const void * ptr);

/* You may make reasonable assumptions or simplifications to simplify your task */

/* Document any assumptions/simplifications in code */

/* You must create a private github/bitbucket repo when you receive this task  */
/* and give us access to that repo. */

/* All your work shall be updated in that repo throughout the period of your task. */

/* Commit regularly to that repo to enable us to track your progress. */

/* Your code should compile without any warnings using gcc -Wall -Wextra -ansi -pedantic -I. test.c */
/*  */

#endif
