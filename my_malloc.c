#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> /* srbk() */
#include <malloc.h> /* mallinfo{}, */
#include <sys/mman.h> /* mmap(), munmap() */
#include <pthread.h>

#define TOTAL_SIZE  (PAGE_NUM * PAGESIZE)
#define PAGESIZE 	4096
#define PAGE_NUM	5 
#define NUM_POINTERS	10

#define HEAP_MAX_SIZE (1024 * 1024)

/* If this bit is 0, the chunk comes from the main arena and the main heap. 
 * If this bit is 1, the chunk comes from mmap'd memory and the location of 
 * the heap can be computed from the chunk's address.  
 */
#define NON_MAIN_ARENA 0x04	
/* this chunk was allocated with mmap and is not part of a heap */
#define IS_MMAPPED 0x02	
/* Previous chunk is in use - if set, and thus the prev_size is invalid */
#define PREV_INUSE 0x01	

#define chunk_non_main_arena(p) ((p)->size & NON_MAIN_ARENA)
#define heap_for_ptr(ptr) \
  ((heap_info *) ((unsigned long) (ptr) & ~(HEAP_MAX_SIZE - 1)))
#  define MALLOC_ALIGNMENT       (2 *SIZE_SZ < __alignof__ (long double)      \
                                  ? __alignof__ (long double) : 2 *SIZE_SZ)

struct malloc_chunk { // 48 bytes in total
  size_t      mchunk_prev_size;  /* Size of previous chunk (valid if free).  */
  /*-------------------------------------------------------------------------*/
  size_t      mchunk_size;       /* Size in bytes, including overhead. */

  /* Fields below is not used for allocated chunk */
  struct malloc_chunk* fd;         /* double links -- used only if free. */
  struct malloc_chunk* bk;

  /* Only used for large blocks: pointer to next larger size.  */
  struct malloc_chunk* fd_nextsize; /* double links -- used only if free. */
  struct malloc_chunk* bk_nextsize;
};

typedef struct malloc_chunk* mchunkptr;
#define SIZE_SZ	(sizeof (size_t))
#define chunk2mem(p)   ((void*)((char*)(p) + 2*SIZE_SZ))
#define mem2chunk(mem) ((mchunkptr)((char*)(mem) - 2*SIZE_SZ))
#define chunkmsg(p)	\
	({\
		void *mem = chunk2mem((p));\
		(char*)(mem + malloc_usable_size(mem) - 32);\
	})


void Usage(char ch)
{
	printf(
	"Usage: [OPTION]... %c?\n"
	"A program to simulate the malloc, free mechanisms\n"
	"  + malloc: m <buffer index> <mem>\n"
	"  + malloc in thread: t <thread index> <mem>\n"
	"  + free  : f <buffer index>\n"
	"  + dump chunk: d <buffer index>\n"
	"  + quit  : q\n"
	"  + list pointers: l\n"
	"  + malloc statistics: s\n"
	"  + malloc info: i\n",
	ch
	);
}

char res[4];
char *bit_chunk(size_t mchunk_size)
{
	res[0] = res[1] = res[2] = '0';
	if (mchunk_size & NON_MAIN_ARENA)
		res[0] = 'A';
	if (mchunk_size & IS_MMAPPED)
		res[1] = 'M';
	if (mchunk_size & PREV_INUSE)
		res[2] = 'P';
	return res;
}

void my_malloc (void **buf, int *is_malloc, int mem, int idx)
{
	if ( !(*is_malloc) ) {
		if ( mem > 0 ) {
			*buf = malloc(mem);
		} else {
			*buf = malloc(TOTAL_SIZE);
		} 

		*is_malloc = 1;
	} else {
		printf("buf[%d] is already malloc'ed\n", idx);
	}
}

void my_free (void *buf, int *is_malloc, int idx)
{
	if ( (*is_malloc) ) {
		free(buf);
		*is_malloc = 0;
		printf("buf[%d] is freed\n", idx);
	}
	else
		printf("buf[%d] isn't malloc'ed\n", idx);
}

struct thread_param
{
	long malloc_size;
	void *addr;
	int index;
};

void* threadFunc(void* arg) {
	struct thread_param *thread = (struct thread_param *) arg;
    char* addr = (char*) malloc(thread->malloc_size);
    fprintf(stderr, "[thread %d] Allocated %ld bytes at %p\n", 
    	thread->index, thread->malloc_size, addr);
    thread->addr = addr;
    return (void *)thread;
    //pthread_exit(thread);
}

int main()
{
	char ch;
	int idx, mem, i;
	void *buf [NUM_POINTERS];
	int check_malloc [NUM_POINTERS] = {0};
	mchunkptr p;
	struct mallinfo mi;
	pthread_t tid[40] = {0};
	struct thread_param thread[40];

	while(scanf("%c", &ch)) 
	{
		if('m'==ch)
		{
			scanf("%d", &idx);
			if( idx >= 0 && idx < NUM_POINTERS ) 
			{
				scanf("%d", &mem);
				my_malloc( &buf[idx], &check_malloc[idx], mem, idx );		

				if(buf[idx])
				{
					printf("- Allocated buf[%d] at %p, usable size: %lu\n",
						idx, buf[idx], malloc_usable_size(buf[idx]));
				} else 
					perror("malloc");
			}
			else
				printf("index (%d) must be in [0,10)\n", idx);
		} else if ('d'==ch) {

			scanf("%d", &idx);

			p = mem2chunk (buf[idx]);
			printf( "  chunk:%p -> previous size: %lu, size: %lu, "
					"bit: %s, fd: %p, bk: %p, msg: \"%s\"\n",
					p, p->mchunk_prev_size, p->mchunk_size & (~0x7),
					bit_chunk(p->mchunk_size), p->fd, p->bk, chunkmsg(p));

		} else if ('f'==ch) {

			scanf("%d", &idx);
			if( idx >= 0 && idx < NUM_POINTERS ) 
			{
				my_free(buf[idx], &check_malloc[idx], idx);
			} else if (idx == 10) {
				scanf("%d", &i);
				fprintf(stderr, "[thread %d] free at %p\n", i, thread[i].addr);
				free (thread[i].addr);
			} else
				printf("index (%d) must be in [0,10)\n", idx);

		} else if ('q'==ch) {

			printf("Clean remaining memory...\n");
			for (i = 0; i < NUM_POINTERS; ++i)
				if ( check_malloc[i] ) 
				{
					free (buf[i]);
					printf("free buf[%d]\n", i);
				}
			break;

		} else if ('s'==ch) {

        	printf("+ program break at %#x\n", sbrk(0));
        	printf("+ malloc_stats(): \n");
        	malloc_stats();

        } else if ('i'==ch) {

        	mi = mallinfo();
        	tinht25();

    	} else if ('l'==ch) {

    		printf("check_malloc[%d] = [ ", NUM_POINTERS);
    		for (i = 0; i < NUM_POINTERS; ++i)
    			printf("%d ", check_malloc[i]);
    		printf("]\n");

    	} else if ('t'==ch) {

    		scanf("%d", &i);
    		thread[i].index = i;
    		scanf("%d", &mem);
    		thread[i].malloc_size = mem;
    		if ( (pthread_create (&tid[i], NULL, threadFunc, (void *) &thread[i])) )
    			fprintf(stderr, "[thread %d] creation error\n", i);

    	} else if ('\n'==ch) {
			continue;
    	} else if ('a'==ch) {

    		FILE *fp = fopen("./dump_inuse_chunk.txt", "w");

    		dump_inuse_chunk(stderr);

    		fclose(fp);
    	
    	} else {
			Usage(ch);
		}
	}

	for (int i = 0; i < 40 && tid[i] != 0; i++) {
		fprintf(stderr, "joining thread %d\n", i);
		pthread_join(tid[i], NULL);
	}
	return 0;
}
