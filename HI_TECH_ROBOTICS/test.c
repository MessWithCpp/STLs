#include "test.h"
#include <stdio.h>
#include <stdbool.h>

/* the heap has a size of 4 MiB. */

/* Your task is to implment a dynamic memory allocator */

/* As a simplification, you may assume that all allocations will be */
/* of the size 16 bytes or smaller */

/* As a debugging aid, printf() may be used */

/* Other than printf(), you may not use any C standard library function or any other code */

/* If you need any auxiliary memory storage for your work,*/
/* then that memory storage shall be from this heap buffer only */
/* NOTE that this may reduce the effective heap size from the application's point of view */

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
	while(temp_node != free_tail)
	{
		if(temp_node->size >= size_requested)
		{
			got_chunk = true;	
			break;
		}
		temp_node = temp_node->next;
	}

	if(free_tail && got_chunk == false)
	{
		if(free_tail->size >= size_requested) {
			temp_node = free_tail;
			got_chunk = true;
		}
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
	struct malloc_chunk_header *next_chunk = NULL;
	struct malloc_chunk_header *next_to_next_chunk = NULL;
	
	if(!temp_node)
		return reconsolidation_successful;
	while(temp_node != free_tail)
	{
		if(temp_node->next)
		{
			/*Checking for 2 contiguous bins*/
			if(((uint8_t*)(temp_node + 1) + (temp_node->size)) == (uint8_t*)(temp_node->next))
			{
				if((temp_node->size + sizeof(struct malloc_chunk_header) + temp_node->next->size) > user_size)
				{	
					temp_node->size = (temp_node->size + sizeof(struct malloc_chunk_header) + temp_node->next->size);
					next_to_next_chunk = temp_node->next->next;
					temp_node->next = next_to_next_chunk;
					reconsolidation_successful = true;
					break;
				}
			}
		}
		temp_node = temp_node->next;
	}
	return reconsolidation_successful;
}

bool check_for_user_chunk(const void *user_ptr)
{
	struct malloc_chunk_header *actual_user_chunk = (struct malloc_chunk_header*)(user_ptr) - 1;
	struct malloc_chunk_header *temp_chunk_node = alloca_head;
	bool got_chunk = false;

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

	return got_chunk;	
}

bool remove_chunk_free_heap(const void *user_chunk_ptr)
{
	struct malloc_chunk_header *user_chunk = (struct malloc_chunk_header*)(user_chunk_ptr) - 1;
	bool is_top_chunk = false;
	struct malloc_chunk_header *temp_chunk = alloca_head;
	
	if((uint8_t*)(user_chunk) + sizeof(struct malloc_chunk_header) + user_chunk->size == heap_top)
	{
		is_top_chunk = true;

		/*If there was just single chunk in list and it was only the top chunk of heap*/
		if(alloca_head == alloca_tail)
		{
			alloca_head = NULL;
			alloca_tail = NULL;
			remaining_heap = HEAP_SIZE;
			heap_top = heap_buffer;
			return is_top_chunk;
		}

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
		heap_top = (uint8_t*)(alloca_tail) + alloca_tail->size;
	}
	return is_top_chunk;
}

void remove_chunk_from_alloca_list(const void *user_chunk_ptr)
{
	struct malloc_chunk_header *actual_chunk = (struct malloc_chunk_header*)(user_chunk_ptr) - 1;
	struct malloc_chunk_header *temp_chunk = alloca_head;
	bool got_chunk = false;
	if(!temp_chunk)
		return;
	while(temp_chunk != alloca_tail)
	{
		if(temp_chunk->next == actual_chunk)
		{
			got_chunk = true;
			break;
		}
		temp_chunk = temp_chunk->next;
	}
	if(got_chunk)
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
		heap_top = (uint8_t*)((struct malloc_chunk_header*)(heap_top) + 1) + n;
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
		remaining_heap = remaining_heap - (sizeof(struct malloc_chunk_header) + n);
		heap_top = (uint8_t*)((struct malloc_chunk_header*)(heap_top) + 1) + n;
		return (void*)(chunk_to_add + 1);
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
	}
	else
	{
		temp_chunk = free_head;
		while(temp_chunk != free_tail)
			temp_chunk = temp_chunk->next;
		temp_chunk->next = (struct malloc_chunk_header*)(ptr) - 1;
		free_tail = temp_chunk->next;
	}		
	return;			
}

/*Do test with below main*/
int main()
{
#if 0
	/*Test case 1 : Basic malloc and free functionality*/
	int *p = (int*)custom_malloc(sizeof(int));
	*p = 55;
	printf("Data : %d\n",*p);
	custom_free(p);
	return 0;
#endif
	/*Test case 2 : Array of pointers using malloc to store address*/
#if 0
	int *arr[10];
	int p = 1;
	for(int i = 0 ; i < 10 ; i++)
	{
		arr[i] = (int*)custom_malloc(sizeof(int));
		printf("Adrresses got : %lx\n",arr[i]);
		if(!arr[i]) {
			printf("Failed malloc\n");
			return 0;
		}
	}
	for(int i = 0 ; i < 10 ; i++)
	{
		*arr[i] = p;
		printf("Data kept : %d at address : %lx\n",p,arr[i]);
		p++;
	}
	for(int i = 0 ; i < 10 ; i++)
	{
		printf("Data fetched : %d at address : %lx\n",*arr[i],arr[i]);
	}
#endif
#if 0
	/*Test case 3 : Continuous malloc and free*/
	int *arr[10];
	int p = 1;
	for(int i = 0 ; i < 10 ; i++)
	{
		arr[i] = (int*)custom_malloc(sizeof(int));
		printf("Adrresses got : %lx\n",arr[i]);
		if(!arr[i]) {
			printf("Failed malloc\n");
			return 0;
		}
		*arr[i] = p++;
		custom_free(arr[i]);
	}
	for(int i = 0 ; i < 10 ; i++)
	{
		printf("Data fetched : %d\n",*arr[i]);
	}
#endif
#if 0
	/*Test case 4 : Heap exhaust*/
	while(1)
	{
		void *ptr = custom_malloc(15);
		if(!ptr)
		{
			printf("Malloc failed\n");
			break;
		}
		else
			printf("Address got : %lx and remaining heap : %d\n",ptr,remaining_heap);
	}
#endif
	/*Test case 5 : Testing reconsolidation of small chunks*/
#if 0
	int *arr[10];
	int p = 1;
	for(int i = 0 ; i < 10 ; i++)
	{
		arr[i] = (int*)custom_malloc(sizeof(int));
		if(!arr[i])
			printf("Malloc failure\n");
		*arr[i] = p++;
	}

	/*Free centre contiguous 2 chunks*/
	custom_free(arr[5]);
	custom_free(arr[6]);

	int *ptr1 = (int*)custom_malloc(8);
	if(ptr1)
	{
		*ptr1 = 999;
	}
	printf("Data stored : %d\n",*ptr1);
#endif
	return 0;
}
/* Compiling this file as it is gives some unused variable errors. */

/* Your final code must not give any warnings */

/* You shall not use C++ for this task, */

/* This task must be done using C language only. */
