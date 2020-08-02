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
#define FACTOR 2
#define MINIMUM_SIZE ((SIZE_T_SIZE)*4)

#define PUT_PTR(p, ptr) ((*(void **)(p)) = ptr)
#define GET_PTR(bp) (*(void **)(bp))
#define GET_PREV(bp) GET_PTR((bp))
#define GET_NEXT(bp) GET_PTR((bp + SIZE_T_SIZE))
#define MAX(a, b) ((a > b)? (a) : (b))



/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{

    //virtual_end = mem_heap_hi();
    int total_size = SIZE_T_SIZE * 6;
    void *heap_start = mem_sbrk(total_size);

    PUT(heap_start, PACK(SIZE_T_SIZE * 4, 1));
    PUT_PTR(heap_start + SIZE_T_SIZE, heap_start + SIZE_T_SIZE);
    PUT_PTR(heap_start + SIZE_T_SIZE * 2, heap_start + SIZE_T_SIZE);
    PUT((heap_start + SIZE_T_SIZE * 3), PACK(SIZE_T_SIZE * 4, 1));

    PUT((heap_start + SIZE_T_SIZE * 4), PACK(SIZE_T_SIZE * 2, 1));
    PUT((heap_start + SIZE_T_SIZE * 5), PACK(SIZE_T_SIZE * 2, 1));

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

void append_list(void *bp)
{
    void *sentinel = mem_heap_lo() + SIZE_T_SIZE;
    void *original_first = GET_PTR(sentinel + SIZE_T_SIZE);
    PUT_PTR(sentinel + SIZE_T_SIZE, bp);  
    PUT_PTR(original_first, bp);
    PUT_PTR(bp, sentinel);
    PUT_PTR(bp + SIZE_T_SIZE, original_first);
}

void delete_list(void *bp)
{
    /*
    * a free block is allocated
    * and should be deleted from the free list
    */
   void *prev_node = GET_PREV(bp);
   void *next_node = GET_NEXT(bp);
   PUT_PTR((prev_node + SIZE_T_SIZE), next_node);
   PUT_PTR(next_node, prev_node);
}

void *extend_heap(size_t alloc_size);

void *first_fit(size_t alloc_size)
{ 
    /*
    * return header pointer instead of base pointer
    */
    
    void *list_start = mem_heap_lo() + SIZE_T_SIZE;
    void *temp_ptr = GET_NEXT(list_start);

    while (temp_ptr != list_start) {
        int size = GET_SIZE(HDRP(temp_ptr));
        int alloc = GET_ALLOC(HDRP(temp_ptr));
        if (alloc == 0 && size >= alloc_size) {
            delete_list(temp_ptr);
            return (temp_ptr - SIZE_T_SIZE);
        }
        temp_ptr = GET_NEXT(temp_ptr);
    }
    if (temp_ptr == list_start) {
        void *ptr = extend_heap(alloc_size * FACTOR);
        return ptr;
    }
    return NULL;
}

void *best_fit(size_t alloc_size)
{
    void *list_start = mem_heap_lo() + SIZE_T_SIZE;
    void *temp_ptr = GET_NEXT(list_start);
    size_t min_size = 0x7fffffff;
    void *min_ptr = NULL;
    while (temp_ptr != list_start) {
        int size = GET_SIZE(HDRP(temp_ptr));
        
        if (size == alloc_size) {
            delete_list(temp_ptr);
            return (temp_ptr - SIZE_T_SIZE);
        }
        if (size > alloc_size) {
            if (size < min_size) {
                min_size = size;
                min_ptr = temp_ptr;
            }
        }
        temp_ptr = GET_NEXT(temp_ptr);
    }
    if (min_ptr == NULL) {
        void *ptr = extend_heap(alloc_size * FACTOR);
        return ptr;
    } else {
        delete_list(min_ptr);
        return (min_ptr - SIZE_T_SIZE);
    }
    return NULL;
}


void *coalesce(void *ptr);
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

    void *returned_ptr = coalesce(old_brk + SIZE_T_SIZE);
    return returned_ptr;
}

void print_list()
{
    void *list_start = mem_heap_lo() + SIZE_T_SIZE;
    void *temp_ptr = GET_NEXT(list_start);
    printf("node address is %p, size is %d and alloc is %d\n",list_start-SIZE_T_SIZE,32,1);
    while (temp_ptr != list_start) {
        int size = GET_SIZE(HDRP(temp_ptr));
        int alloc = GET_ALLOC(HDRP(temp_ptr));
        printf("node address is %p, size is %d and alloc is %d\n", 
        (temp_ptr - SIZE_T_SIZE), size, alloc);
        temp_ptr = GET_NEXT(temp_ptr);
    }

}

void *find_fit(size_t alloc_size) 
{
    /*
    * input is allocation size
    * output is header pointer instead of base pointer
    */
    //void * ptr = first_fit(alloc_size);
    void *ptr = best_fit(alloc_size);
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

    size_t newsize = MAX(ALIGN(size + SIZE_T_SIZE * 2), MINIMUM_SIZE);
    void *ptr = find_fit(newsize);
    delete_list(ptr + SIZE_T_SIZE);
    size_t oldsize = GET_SIZE(ptr);
    int delta_size = oldsize - newsize;
    if (delta_size < MINIMUM_SIZE) {
        newsize = oldsize;
    }
    PUT(ptr, PACK(newsize, 1));
    void *footer = (void *)((char *)ptr + newsize - SIZE_T_SIZE); // void ptr arithmetic undefined
    PUT(footer, PACK(newsize, 1));
    
    if (delta_size >= MINIMUM_SIZE) {
        // pay attention to the equal sign or it will cause bugs!
        // splitting!
        PUT(footer + SIZE_T_SIZE, PACK(delta_size, 0));
        PUT(footer + delta_size, PACK(delta_size, 0));
        append_list(footer + SIZE_T_SIZE * 2);
    }
    //printf("malloc : %d\n", size);
    /*
    printf("malloc : %d\n", size);
    print_list();
    print_heap();
    */
    return (void *) ((char *)ptr + SIZE_T_SIZE);
}

void *coalesce(void *ptr)
{
    /*
    * intput is base pointer
    * output is header pointer pointing to the start of that region
    * 
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
        append_list(ptr);
        return header;
    }

    if (prev_alloc && !next_alloc) {
        void *next_footer = next_header + GET_SIZE(next_header) - SIZE_T_SIZE;
        int new_size = size + GET_SIZE(next_header);
        PUT(header, PACK(new_size, 0));
        PUT(next_footer, PACK(new_size, 0));
        delete_list(next_header + SIZE_T_SIZE);
        append_list(ptr);
        return header;
    }

    if (!prev_alloc && next_alloc) {
        void *prev_header = prev_footer - GET_SIZE(prev_footer) + SIZE_T_SIZE;
        int new_size = size + GET_SIZE(prev_footer);
        PUT(footer, PACK(new_size, 0));
        PUT(prev_header, PACK(new_size, 0));
        delete_list(prev_header + SIZE_T_SIZE);
        append_list(prev_header + SIZE_T_SIZE);
        return prev_header;
    }

    if (!prev_alloc && !next_alloc) {
        void *prev_header = prev_footer - GET_SIZE(prev_footer) + SIZE_T_SIZE;
        void *next_footer = next_header + GET_SIZE(next_header) - SIZE_T_SIZE;
        int new_size = size + GET_SIZE(next_header) + GET_SIZE(prev_footer);
        PUT(prev_header, PACK(new_size, 0));
        PUT(next_footer, PACK(new_size, 0));
        delete_list(prev_header + SIZE_T_SIZE);
        delete_list(next_header + SIZE_T_SIZE);
        append_list(prev_header + SIZE_T_SIZE);
        return prev_header;
    }
    return NULL;
    
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    coalesce(ptr);

    //printf("free %p\n", ptr);
    /*
    printf("after free is :\n");
    print_list();
    print_heap();
    */
       
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


void *mm_realloc_old(void *ptr, size_t size)
{
    void *oldptr = ptr;
    int old_size = GET_SIZE(HDRP(ptr));
    
    int actual_size = MAX(ALIGN(size) + SIZE_T_SIZE*2, MINIMUM_SIZE);
    //printf("realloc oldsize is %d and size is %d\n", old_size, size);
    if (actual_size == old_size) {
        return ptr;
    }
    
    void *old_header = HDRP(ptr);
    void *old_footer = FTRP(ptr);
    
    if (old_size >= actual_size + MINIMUM_SIZE) {
        void *new_footer = old_header + actual_size - SIZE_T_SIZE;    
        PUT(old_header, PACK(actual_size, 1));
        PUT(new_footer, PACK(actual_size, 1));
        int delta_size = old_size - actual_size;
        PUT(old_header+actual_size, PACK(delta_size, 0));
        PUT(old_footer, PACK(delta_size, 0));
        // TODO
        coalesce(ptr + actual_size);
        return ptr;
    }
    if (old_size >= actual_size && old_size < actual_size + MINIMUM_SIZE) {
        return ptr;
    }
    if (old_size < actual_size) {
        void *newptr = mm_malloc(size);
        if (newptr == NULL)
            return NULL;
        memcpy(newptr, oldptr, size); 
        mm_free(oldptr);
        return newptr;
    }

    return NULL;

}














