/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "ateam",
    /* First member's full name */
    "Harry Bovik",
    /* First member's email address */
    "bovik@cs.cmu.edu",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

#define WSIZE 4
#define DSIZE 8
#define PACK(size, alloc) ((size) | (alloc))
#define HDRP(bp) ((char *)bp - (SIZE_T_SIZE))


#define GET_SIZE(p) ((*(size_t *)(p)) & ~0x7)
#define GET_ALLOC(p) ((*(size_t *)(p)) & 0x1)

#define FTRP(bp) ((char *)bp + GET_SIZE(HDRP(bp)) - (2*SIZE_T_SIZE))
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE((char *)(bp) - (SIZE_T_SIZE*2)))
#define PUT(p, val) ((*(size_t *)(p)) = (val))



/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{

    //virtual_end = mem_heap_hi();
    int total_size = SIZE_T_SIZE * 4;
    void *heap_start = mem_sbrk(total_size);
    PUT(heap_start, PACK(16, 1));
    PUT((heap_start + SIZE_T_SIZE), PACK(16, 1));
    PUT((heap_start + SIZE_T_SIZE * 2), PACK(16, 1));
    PUT((heap_start + SIZE_T_SIZE * 3), PACK(16, 1));


    return 0;
}


void print_heap() {
    char * ptr = mem_heap_lo();
    void *physical_boundary = mem_heap_hi() + 1 - SIZE_T_SIZE * 2;
    while (ptr < (char *)physical_boundary) {
        int size = GET_SIZE(ptr);
        int alloc = GET_ALLOC(ptr);
        
        printf("address is %p\n", (void *)ptr);
        printf("size : %d alloc : %d\n", size, alloc);
        printf("---------------\n");
        ptr += size;
        if (size == 0) {
            break;
        }
    }
    printf("====================\n");

}
void coalesce(void *ptr);
void *extend_heap(size_t alloc_size)
{
    void *old_brk = mem_sbrk(alloc_size);
    if (old_brk == (void *)(-1)) {
        printf("mem_brk error\n");
        exit(1);
    }
    old_brk = (char *)old_brk - SIZE_T_SIZE * 2;
    PUT(old_brk, PACK(alloc_size, 0));
    void *footer = FTRP(old_brk + SIZE_T_SIZE);
    PUT(footer, PACK(alloc_size, 0));
    PUT(footer + SIZE_T_SIZE, PACK(16,1));
    PUT(footer + SIZE_T_SIZE * 2, PACK(16, 1));

    coalesce(old_brk + SIZE_T_SIZE);
    return old_brk;
}


void *first_fit(size_t alloc_size)
{
    
    char * ptr = mem_heap_lo() + SIZE_T_SIZE * 2;
    void *physical_boundary = mem_heap_hi() + 1 - SIZE_T_SIZE * 2; // (brk - 1) + 1
    int delta = (char *)(physical_boundary) - ptr;
    if (delta == 0) {
        void * old_brk = extend_heap(alloc_size * 2);
        return old_brk;
    }
    while (1)
    {
        // if delta > 0
        int size = GET_SIZE(ptr);
        int alloc = GET_ALLOC(ptr);
        if (alloc == 1) {
            if (ptr == physical_boundary) {
                extend_heap(alloc_size * 2);
                return ptr;
            }
            ptr += size;
            continue;
        }
        if (alloc == 0) {
            if (size >= alloc_size) {
                return (void *) ptr;
            } else {
                void *footer = ptr + GET_SIZE(ptr) - SIZE_T_SIZE;
                if (footer + SIZE_T_SIZE == physical_boundary) {
                    int delta_size = (ptr + alloc_size) - (char *)(physical_boundary);
                    extend_heap(delta_size * 2);
                    
                    return ptr;
                }
                //printf("ptr is %p and size is %d\n", ptr, size);
                ptr += size;
            }
        }
        
    }
    
}

void *find_fit(size_t alloc_size) 
{
    void * ptr = first_fit(alloc_size);
    //void * ptr = second_fit(alloc_size);
    if (ptr == NULL) {
        printf("stack overflow\n");
        exit(1);
    }
    return ptr;
}


/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    if (size == 0) {
        return NULL;
    }

    size_t newsize = ALIGN(size + SIZE_T_SIZE * 2);
    void *ptr = find_fit(newsize);
    size_t oldsize = GET_SIZE(ptr);
    int delta_size = oldsize - newsize;
    PUT(ptr, PACK(newsize, 1));
    void *footer = (void *)((char *)ptr + newsize - SIZE_T_SIZE); // void ptr arithmetic undefined
    PUT(footer, PACK(newsize, 1));
    
    if (delta_size > 0) {
        // splitting!
        PUT(footer + SIZE_T_SIZE, PACK(delta_size, 0));
        PUT(footer + delta_size, PACK(delta_size, 0));
    }
    

    return (void *) ((char *)ptr + SIZE_T_SIZE);
}

void coalesce(void *ptr)
{
    /*
    * if we didn't introduce sentinel node
    * then there will be many corner cases
    * e.g.
    * the block didn't have the prev one but have the next one
    * the block didn't have the prev one and didn't have the next one
    * the block have the prev one but didn't have the next one
    * the block have the prev one and have the next one
    */
    void *header = HDRP(ptr);
    void *footer = FTRP(ptr);
    int size = GET_SIZE(HDRP(ptr));

    void *prev_footer = ptr - SIZE_T_SIZE * 2;
    int prev_alloc = GET_ALLOC(prev_footer);

    void *next_header = footer + SIZE_T_SIZE;
    int next_alloc = GET_ALLOC(next_header);

    if (prev_alloc && next_alloc) {
        PUT(header, PACK(size, 0));
        PUT(footer, PACK(size, 0));
    }

    if (prev_alloc && !next_alloc) {
        void *next_footer = next_header + GET_SIZE(next_header) - SIZE_T_SIZE;
        int new_size = size + GET_SIZE(next_header);
        PUT(header, PACK(new_size, 0));
        PUT(next_footer, PACK(new_size, 0));
    }

    if (!prev_alloc && next_alloc) {
        void *prev_header = prev_footer - GET_SIZE(prev_footer) + SIZE_T_SIZE;
        int new_size = size + GET_SIZE(prev_footer);
        PUT(footer, PACK(new_size, 0));
        PUT(prev_header, PACK(new_size, 0));
    }

    if (!prev_alloc && !next_alloc) {
        void *prev_header = prev_footer - GET_SIZE(prev_footer) + SIZE_T_SIZE;
        void *next_footer = next_header + GET_SIZE(next_header) - SIZE_T_SIZE;
        int new_size = size + GET_SIZE(next_header) + GET_SIZE(prev_footer);
        PUT(prev_header, PACK(new_size, 0));
        PUT(next_footer, PACK(new_size, 0));
    }

    
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    coalesce(ptr);   
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;

    copySize = GET_SIZE((char *)(oldptr - SIZE_T_SIZE));
    if (size == copySize) {
        return ptr;
    }
    
    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;
    if (size < copySize) {
        copySize = size;
    }
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;

    
}














