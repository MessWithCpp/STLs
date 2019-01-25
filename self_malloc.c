#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/mman.h>

#define M_MMAP_THRESHOLD_ASH 128*1024
#define PAGE_SIZE_ASH 4*1024

struct malloc_chunk_header
{
	size_t size;
	bool is_free;
	struct malloc_chunk_header *next;
};

struct mmapped_chunk_header
{
	size_t size;
	struct mmapped_chunk *next;
};

struct malloc_chunk_header *head , *tail;

pthread_mutex_t malloc_lock;

struct malloc_chunk_header* get_free_chunk(size_t user_req_size)
{
	struct malloc_chunk_header *temp = head;
	while(temp != NULL)
	{
		if(temp->is_free == true && (temp->size) >= user_req_size)
			break;
		temp = temp->next;
	}
	return temp;
}
 
void* ash_malloc(size_t size)
{
	struct malloc_chunk_header *header;
	struct mmapped_chunk_header *mmap_header;
	size_t total_size;
	void *block;
	if(!size)
		return NULL;

	/*Checking size for making sbrk or mmap*/
	if(n > M_MMAP_THRESHOLD_ASH)
	{
		size = size + 
		size_t len = (size & ~(PAGE_SIZE_ASH - 1)) + PAGE_SIZE_ASH;
		char *base = __mmap(0, len, PROT_READ|PROT_WRITE,
                   		    MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
		if (base == (void *)-1) return NULL;
		
	}
	/*Taking lock to protect thread from accessing head and tail memory*/
	pthread_mutex_lock(&malloc_lock);
	
	/*Bring block from library bin if possible*/
	header = get_free_chunk(size);

	if(header)
	{
		header->is_free = false;
		pthread_mutex_unlock(&malloc_lock);
		return (void*)(header + 1);
	}

	/*If library has no such chunk of desired size , code comes here*/
	total_size = size + sizeof(struct malloc_chunk_header);
	
	/*Request OS for heap memory using sbrk*/
	block = sbrk(total_size);

	if(block == (void*)-1)
	{
		pthread_mutex_unlock(&malloc_lock);
		return NULL;
	}

	header = block;
	header->is_free = false;
	header->size = size;
	if(!head) {
		head = header;
		head->next = NULL;
	}
	if(tail)
	{
		tail->next = header;
		header->next = NULL;
	}
	tail = header;
	pthread_mutex_unlock(&malloc_lock);
	return (void*) (header + 1);
}

int main()
{
	int *ptr = (int*)ash_malloc(sizeof(int));
	int *ptr_arr[10] = {0};
	int index = 0;
	int data = 1;

	/*Allocate*/
	for(index = 0 ; index < 10 ; index++)
	{
		ptr_arr[index] = (int*)ash_malloc(sizeof(int));
	}

	/*Write data*/
	for(index = 0 ; index < 10 ; index++)
	{
		ptr_arr[index] = data++;
	}
	
	/*Read data*/
	for(index = 0 ; index < 10 ; index++)
	{
		printf("Data in index %d is %d\n",index,ptr_arr[index]);
	}
	return 0;
}


