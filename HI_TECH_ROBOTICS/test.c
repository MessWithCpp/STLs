/* Author : Amanullah Ashraf
 * Target : Making custom malloc much original like original.
 * */

/*In this whole code I have tried my best to let malloc() behave like original malloc
 *Please do read the comments in relation to each step I have taken */

#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/mman.h>

#define HEAP_SIZE (4 * 1024 * 1024) /* I have not restricted the code to just 4 MB. You can request the system as much 
				       you want */

#define M_MMAP_THRESHOLD_ASH 128*1024 /*Kept the mmap() threshold just to make difference for mmap() and sbrk() syscalls*/
#define PAGE_SIZE_ASH 4*1024 /*Kept the page size as like x86 on standard malloc() i.e. 4k*/
struct malloc_chunk_header *head , *tail; /*Keeping the addresses of chunks recieved from sbrk() in library itself*/
pthread_mutex_t malloc_lock; /*Locking variable for protecting head and tail global variables*/

/* Below is the structure of actual chunk that I made to keep the header 
 * information of memory fetched either from mmap or sbrk syscalls*/
struct malloc_chunk_header
{
	size_t size;
	bool is_free;
	struct malloc_chunk_header *next;
};

/* the heap has a size of 4 MiB. */

/* Your task is to implment a dynamic memory allocator */

/* As a simplification, you may assume that all allocations will be */ /*----> I have not taken this assumption. Ask as much as you wantfrom this custom malloc()*/
/* of the size 16 bytes or smaller */

/* As a debugging aid, printf() may be used */

/* Other than printf(), you may not use any C standard library function or any other code */

/* If you need any auxiliary memory storage for your work,*/
/* then that memory storage shall be from this heap buffer only */
/* NOTE that this may reduce the effective heap size from the application's point of view */

//static uint8_t heap_buffer[HEAP_SIZE];

/* this file MUST NOT expose any symbols other than  */
/* custom_free AND custom_malloc */

/* You may not use any other dynamic memory allocator in this file */

/* You may not use any memory outside the heap_buffer */

/* This function get_free_chunk() gives the addresses of malloc chunk that is already present in library after syscall sbrk()
 * This is basically checking the user requested size and comparing it with the existing size freed chunks
 * present in library and then returns if  successfully got it there in linked list */

struct malloc_chunk_header* get_free_chunk(size_t user_req_size)
{
	bool got_chunk = false;
	struct malloc_chunk_header *temp = head;

	/*Traversing the sbrk() recievd chunk list to get desired chunk size*/
	while(temp != NULL)
	{
		if(temp->is_free == true && (temp->size) >= user_req_size)
		{
			got_chunk = true;
			break;
		}
		temp = temp->next;
	}
	if(got_chunk == true)
		return temp;
	else
		return NULL;
}

/*Actual code of malloc*/
void * custom_malloc(size_t size)
{
	struct malloc_chunk_header *header;
	struct mmapped_chunk_header *mmap_header;
	size_t total_size;
	void *block;

	/*Returning NULL for zero size*/
	if(!size)
		return NULL;

	/*Checking size for making sbrk or mmap*/
	/*For size requested above 128 kb we are making mmap() syscall*/
	if(size > M_MMAP_THRESHOLD_ASH)
	{
		/*Making size as page aligned for successful mmap()*/
		/*Page size taken here is as like x86 page size 4 kb*/
		size_t len = (size & ~(PAGE_SIZE_ASH - 1)) + PAGE_SIZE_ASH;
		char *base = mmap(0, len, PROT_READ|PROT_WRITE,
				MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
		if (base == (void *)-1)
		{	
			printf("MMAP failed\n");
			return NULL;
		}

		/* Typecasting address recieved to write header information
		 * and then returning the user , address(header+1) i.e address
		 * after header information where user application can write*/

		header = (struct malloc_chunk_header*)(base);
		header->size = size;
		return (void*)(header + 1);
	}
	/*Taking lock to protect thread from accessing head and tail memory*/
	pthread_mutex_lock(&malloc_lock);

	/*Bring block from self designed library bin if possible*/
	/*Please read the functionality of this function above this
	 *functions definition */
	header = get_free_chunk(size);

	if(header)
	{
		header->is_free = false;
		pthread_mutex_unlock(&malloc_lock);
		return (void*)(header + 1);
	}

	/*If library has no such sbrk() chunk of desired size , code comes here*/

	/*Total size taken here is user size + header size to keep header info of chunk*/
	total_size = size + sizeof(struct malloc_chunk_header); 

	/*Request OS for heap memory using sbrk syscall*/
	block = sbrk(total_size);

	if(block == (void*)-1)
	{
		pthread_mutex_unlock(&malloc_lock);
		return NULL;
	}

	header = block;
	header->is_free = false;
	header->size = size;

	/*Maintaining a single link list below for function get_free_chunk()*/

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

	/*Returning actual pointer just after the point where header is finished
	 *Thats why returned (header+1) */
	return (void*) (header + 1);
}

void custom_free(const void * ptr)
{
	struct malloc_chunk_header *header;

	/* Typecasting the void pointer to chunk so that we can
	 * do arithmatic operations on pointer as we did in malloc*/
	header = (struct malloc_chunk_header*)(ptr);

	/* Coming back to the pointer address where actual
	 * memory was allocated . In malloc we returned the user 
	 * header + 1 , so here we need to subtract back*/

	header = header - 1;

	/* Making size comparison to take decision whether to keep
	 * chunk in library link list as free or to release it to 
	 * system using munmap() */

	if(header->size > M_MMAP_THRESHOLD_ASH)
	{
		/*Frees the memory allocated using mmap*/
		size_t size_loc = header->size;
		size_t len = (size_loc & ~(PAGE_SIZE_ASH - 1)) + PAGE_SIZE_ASH;
		int ret = munmap(header , len);
		if(ret == -1)
		{
			printf("MUNMAP failure\n");
		}
		return;
	}
	else
		/* Just marks the memory as free and keep them in library for later usage
		 * while malloc() */
		header->is_free = true;
}

int main()
{
	/*Test your sbrk code part of malloc size here*/
	int *ptr = (int*)custom_malloc(sizeof(int));
	*ptr = 10;
	printf("Memory accessed from sbrk call and its data is : %d\n",*ptr);
	custom_free(ptr);

	/*Test your mmap code part of malloc here*/
	void *ptr1 = custom_malloc(129*1024);
	ptr = (int*)(ptr1);
	*ptr = 1000;
	printf("Memory accessed from mmap syscall and its data is : %d\n",*ptr);
	custom_free(ptr);
	return 0;
}
