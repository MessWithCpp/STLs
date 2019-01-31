#include "test.h"
#include <stdio.h>
#include <stdbool.h>

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

/* this file MUST NOT expose any symbols other than  */
/* custom_free AND custom_malloc */

/* You may not use any other dynamic memory allocator in this file */

/* You may not use any memory outside the heap_buffer */

void malloc_error(const char *str)
{
	printf("Malloc error : %s\n",str);
}

struct malloc_chunk_header* search_free_bins(const int32_t size_requested)
{
	struct malloc_chunk_header *temp_node = free_head;
	bool got_chunk = false;

	if(!temp_node)
		return NULL;
	while(temp_node != NULL)
	{
		if(temp_node->size >= size_requested)
		{
			got_chunk = true;	
			break;
		}
		temp_node = temp_node->next;
	}
	if(got_chunk)
		return temp_node;
	else
		return NULL;
}

bool reconsolidate_free_bin(const int32_t user_size)
{
	bool reconsolidation_successful = false;
	struct malloc_chunk_header *temp_node = free_head;
	struct malloc_chunk_next *next_chunk = NULL;
	struct malloc_chunk_next *next_to_next_chunk = NULL;
	
	if(!temp_node)
		return reconsolidation_successful;
	if(temp_node->next)
	{
		/*Checking for 2 contiguous bins*/
		if(((uint8_t*)(temp_node + 1) + temp_node->size) == temp_node->next)
		{
			temp_node->size = (temp_node->size + sizeof(struct malloc_chunk_header) + temp_node->next->size);
			next_to_next_chunk = temp_node->next->next;
			temp_node->next = next_to_next_chunk;
			reconsolidation_successful = true;
		}
	}
	return reconsolidation_successful;
}

bool check_for_user_chunk(struct malloc_chunk_header *user_ptr)
{
	struct malloc_chunk_header *actual_user_chunk = user_ptr - 1;
	struct malloc_chunk_header *temp_chunk_node = alloca_head;
	bool got_chunk = false;

	if(!temp_chunk_node)
		return false;

	/*Searching in alloca bin list*/
	while(temp_chunk_node != NULL)
	{
		if(temp_chunk_node == actual_user_chunk)
		{
			got_chunk = true;
			break;
		}	
		temp_chunk_node = temp_chunk_node->next;	
	}
	if(got_chunk)
		return true;

	temp_chunk_node = free_head;
	if(!temp_chunk_node)
		return false;

	/*Searching is free bin list*/
	while(temp_chunk_node != NULL)
	{
		if(temp_chunk_node == actual_user_chunk)
		{
			got_chunk = true;
			break;
		}	
		temp_chunk_node = temp_chunk_node->next;	
	}
	if(got_chunk)
		return true;

	return got_chunk;	
}

bool remove_chunk_free_heap(struct malloc_chunk_header *user_chunk)
{
	bool is_top_chunk = false;
	struct malloc_chunk_header *temp_chunk = alloca_head;

	if((uint8_t*)(user_chunk) + user_chunk->size == heap_top)
	{
		is_top_chunk = true;

		/*Traverse alloca list free heap*/
		while(temp_chunk->next != alloca_tail)
			temp_chunk = temp_chunk->next;
	}

	/*Free heap if got it as top chunk*/
	if(is_top_chunk)
	{
		temp_chunk->next = NULL;
		alloca_tail = temp_chunk;
		remaining_heap = remaining_heap + sizeof(struct malloc_chunk_header) + user_chunk->size;
		heap_top = (uint8_t*)(user_chunk) + user_chunk->size;
	}
	return is_top_chunk;
}

void remove_chunk_from_alloca_list(struct malloc_chunk_header *user_chunk)
{
	struct malloc_chunk_header *actual_chunk = user_chunk - 1;
	struct malloc_chunk_header *temp_chunk = alloca_head;
	if(!temp_chunk)
		return;
	while(temp_chunk != NULL)
	{
		if(temp_chunk->next == actual_chunk)
			break;
		temp_chunk = temp_chunk->next;
	}
	temp_chunk->next = temp_chunk->next->next;
}

void * custom_malloc(const int32_t n)
{
	struct malloc_chunk_header *lib_bin_ptr = NULL;
	struct malloc_chunk_header *temp_chunk = NULL;
	struct malloc_chunk_header *chunk_to_add = NULL;

	bool reconsolidation_successful = false;

	/* You may return a null pointer if n > MAX_ALLOC */
	if(n > MAX_ALLOC)
		return NULL;

	/*Searching free bins in library*/
	lib_bin_ptr = search_free_bins(n);
	
	if(lib_bin_ptr)
		return (void*)(lib_bin_ptr + 1);

	/*Reconsolidate small sizes chunks in free bins to make large chunk*/
	reconsolidation_successful = reconsolidate_free_bin(n);


	/*Searching free bins again in library after reconsolidation*/
	if(reconsolidation_successful)
	{
		lib_bin_ptr = search_free_bins(n);
	
		if(lib_bin_ptr)
			return (void*)(lib_bin_ptr + 1);
	}

	/*If we are not able to get chunk of desired size then we need to expand heap*/
	if((n > (remaining_heap - sizeof(struct malloc_chunk_header))) || remaining_heap < 0)
	{
		malloc_error("Heap Exhaused\n");
		return NULL;
	}

	/*Time to expand heap*/
	if(!alloca_head)
	{
		temp_chunk = (struct malloc_chunk_header*)(heap_top);
		temp_chunk->size = n;
		temp_chunk->next = NULL;
		alloca_head = temp_chunk;
		alloca_tail = alloca_head;
		remaining_heap = remaining_heap - (sizeof(struct malloc_chunk_header) + n);
		heap_top = (char*)((struct malloc_chunk_header*)(heap_top) + 1) + n;
		return (void*)(alloca_head + 1);
	}
	else
	{
		temp_chunk = alloca_head;
		while(temp_chunk->next != NULL)
			temp_chunk = temp_chunk->next;
		chunk_to_add = (struct malloc_chunk_header*)(heap_top);
		chunk_to_add->size = n;
		chunk_to_add->next = NULL;
		temp_chunk->next = chunk_to_add;
		return (void*)(temp_chunk + 1);
	}	
}

void custom_free(const void * ptr)
{
	bool chunk_is_present = false;
	bool chunk_is_top_most_chunk = false;
	struct malloc_chunk_header *temp_chunk = NULL;

	/*Check if that chunk is even a valid chunk or not*/
	chunk_is_present = check_for_user_chunk(ptr);
	if(!chunk_is_present)
		return;
	
	/*Check if chunk is top most chunk of heap then we free heap by expanding its size*/
	chunk_is_top_most_chunk = remove_chunk_free_heap(ptr);
	if(chunk_is_top_most_chunk)
		return;

	/*Remove the chunk from alloca list by changing links*/
	remove_chunk_from_alloca_list(ptr);

	/*Adding chunk to free bin list after removing from alloca list*/
	if(!free_head)
	{
		free_head = (struct malloc_chunk_header*)(ptr) - 1;
		free_tail = free_head;
		free_head->next = NULL;
	}
	else
	{
		temp_chunk = free_head;
		while(temp_chunk->next != NULL)
			temp_chunk = temp_chunk->next;
		temp_chunk->next = (struct malloc_chunk_header*)(ptr) - 1;
		free_tail = temp_chunk->next;
		free_tail->next = NULL;
	}		
	return;			
}

int main()
{
	int *p = (int*)custom_malloc(sizeof(int));
	*p = 55;
	printf("Data : %d\n",*p);
	return 0;
}
/* Compiling this file as it is gives some unused variable errors. */

/* Your final code must not give any warnings */

/* You shall not use C++ for this task, */

/* This task must be done using C language only. */
