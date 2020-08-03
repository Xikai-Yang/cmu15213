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
/*
 * this version is implemented using segregated list
 * and best-fit method
 * but the util is still the same
 * maybe you have to change the realloc method
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
#define HDRP(bp) ((char *)bp - (WSIZE))


#define GET_SIZE(p) ((*(size_t *)(p)) & ~0x7)
#define GET_ALLOC(p) ((*(size_t *)(p)) & 0x1)

#define FTRP(bp) ((char *)bp + GET_SIZE(HDRP(bp)) - (2*WSIZE))
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE((char *)(bp) - (WSIZE*2)))
#define PUT(p, val) ((*(size_t *)(p)) = (val))
#define FACTOR 2
#define MINIMUM_SIZE ((WSIZE)*4)

#define PUT_PTR(p, ptr) ((*(void **)(p)) = ptr)
#define GET_PTR(bp) (*(void **)(bp))
#define GET_PREV(bp) GET_PTR((bp))
#define GET_NEXT(bp) GET_PTR((bp + WSIZE))


#define MAX(a, b) ((a > b)? (a) : (b))


// 17-32 33-64 65-128 129-256 257-512 513-1024 1025-2048 2049-INF

void init_segblock()
{
    int total_size = WSIZE * 4;
    void *block_start = mem_sbrk(total_size);
    PUT(block_start, PACK(WSIZE * 4, 1));
    PUT_PTR(block_start + WSIZE, block_start + WSIZE);
    PUT_PTR(block_start + WSIZE * 2, block_start + WSIZE);
    PUT(block_start + WSIZE * 3, PACK(WSIZE * 4, 1));
}

void *cal_list_header(size_t size)
{
    void *heap_start = mem_heap_lo();
    if (size >= 16 && size <= 32) {
        return heap_start + WSIZE;
    }
    if (size > 32 && size <= 64) {
        return heap_start + DSIZE*2 + WSIZE;
    }
    if (size > 64 && size <= 128) {
        return heap_start + (DSIZE*2)*2 + WSIZE;
    }
    if (size > 128 && size <= 256) {
        return heap_start + (DSIZE*2)*3 + WSIZE;
    }
    if (size > 256 && size <= 512) {
        return heap_start + (DSIZE*2)*4 + WSIZE;
    }
    if (size > 512 && size <= 1024) {
        return heap_start + (DSIZE*2)*5 + WSIZE;
    }
    if (size > 1024 && size <= 2048) {
        return heap_start + (DSIZE*2)*6 + WSIZE;
    }
    if (size > 2048) {
        return heap_start + (DSIZE*2)*7 + WSIZE;
    }
    return NULL;

}

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init()
{

    //virtual_end = mem_heap_hi();
    size_t seg_counter = 8;
    for (size_t i = 0; i < seg_counter; i++)
    {
        init_segblock();
        /* code */
    }

    

    int total_size = WSIZE * 7;
    void *heap_start = mem_sbrk(total_size);
    void *header = heap_start + WSIZE * 2;

    PUT(heap_start, 0);

    PUT(heap_start + WSIZE, PACK(DSIZE * 2, 1));
    
    PUT_PTR(heap_start + WSIZE * 2, header);
    PUT_PTR(heap_start + WSIZE * 3, header);

    PUT(heap_start + WSIZE * 4, PACK(DSIZE * 2, 1));

    PUT(heap_start + WSIZE * 5, PACK(DSIZE, 1));
    PUT(heap_start + WSIZE * 6, PACK(DSIZE, 1));

    return 0;
}
void print_heap() {
    char * ptr = mem_heap_lo() + WSIZE + DSIZE*2*8;
    void *physical_boundary = mem_heap_hi() + 1 - WSIZE * 2;
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
    size_t free_size = GET_SIZE(HDRP(bp));
    void *sentinel = cal_list_header(free_size);
    void *original_first = GET_NEXT(sentinel);

    PUT_PTR(sentinel + WSIZE, bp);  
    PUT_PTR(original_first, bp);
    PUT_PTR(bp, sentinel);
    PUT_PTR(bp + WSIZE, original_first);
}

void delete_list(void *bp)
{
    /*
    * a free block is allocated
    * and should be deleted from the free list
    */
   void *prev_node = GET_PREV(bp);
   void *next_node = GET_NEXT(bp);
   PUT_PTR((prev_node + WSIZE), next_node);
   PUT_PTR(next_node, prev_node);
}



void *extend_heap(size_t alloc_size);


void *list_search(void *list_start, size_t alloc_size)
{
    // RETURN header pointer
    void *temp_ptr = GET_NEXT(list_start);
    while (temp_ptr != list_start) {
        int size = GET_SIZE(HDRP(temp_ptr));
        int alloc = GET_ALLOC(HDRP(temp_ptr));
        if (alloc == 0 && size >= alloc_size) {
            // delete_list(temp_ptr);
            return (temp_ptr - WSIZE);
        }
        temp_ptr = GET_NEXT(temp_ptr);
    }
    if (temp_ptr == list_start) {
        return NULL;
    }
    return NULL;

}

void *first_fit(size_t alloc_size)
{
    /*
    * return header pointer instead of base pointer
    */
    void *list_start = cal_list_header(alloc_size);
    void *bound = mem_heap_lo() + DSIZE*2*8;
    void *ptr = list_search(list_start, alloc_size);

    while (ptr == NULL && list_start < bound) {
        list_start += DSIZE*2;
        ptr = list_search(list_start, alloc_size);
    }
    if (ptr != NULL) {
        return ptr;
    }
    else if (list_start > bound && ptr == NULL) {
        // how to place new blocks?
        void *ptr = extend_heap(alloc_size * FACTOR);
        return ptr;
    }
    else {
        printf("error!\n");
        exit(1);
    }
    return NULL;
}

void print_heap();
void print_list();
void *list_best_search(void *list_start, size_t alloc_size)
{
    // return header pointer

    
    void *temp_ptr = GET_NEXT(list_start);

    size_t min_size = 0x7fffffff;
    void *min_ptr = NULL;
    while (temp_ptr != list_start) {

        int size = GET_SIZE(HDRP(temp_ptr));
        
        if (size == alloc_size) {
            // delete_list(temp_ptr);
            return (temp_ptr - WSIZE);
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
        return NULL;
    } else {
        //delete_list(min_ptr);
        return (min_ptr - WSIZE);
    }
    return NULL;



}
void *best_fit(size_t alloc_size)
{
    void *list_start = cal_list_header(alloc_size);
    void *bound = mem_heap_lo() + DSIZE*2*8;

    void *ptr = NULL;
    while (ptr == NULL && list_start < bound) {
        
        ptr = list_best_search(list_start, alloc_size);
        list_start += DSIZE*2;
    }
    if (ptr != NULL) {
        return ptr;
    }
    if (ptr == NULL) {
        void *ptr = extend_heap(alloc_size * FACTOR);
        return ptr;
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
    old_brk = (char *)old_brk - WSIZE * 2;
    PUT(old_brk, PACK(alloc_size, 0));

    void *footer = FTRP(old_brk + WSIZE);
    PUT(footer, PACK(alloc_size, 0));

    PUT(footer + WSIZE, PACK(DSIZE,1));
    PUT(footer + WSIZE * 2, PACK(DSIZE, 1));

    void *returned_ptr = coalesce(old_brk + WSIZE);
    return returned_ptr;
}


void print_single_list(void *list_start)
{
    void *temp_ptr = GET_NEXT(list_start);
    int num = (list_start - mem_heap_lo() - WSIZE)/(DSIZE*2);
    printf("this is the %d th list\n", num);
    printf("node address is %p, size is %d and alloc is %d\n",list_start-WSIZE,16,1);
    while (temp_ptr != list_start) {
        int size = GET_SIZE(HDRP(temp_ptr));
        int alloc = GET_ALLOC(HDRP(temp_ptr));
        printf("node address is %p, size is %d and alloc is %d\n", 
        (temp_ptr - WSIZE), size, alloc);
        temp_ptr = GET_NEXT(temp_ptr);
    }

}

void print_list()
{
    void *list_start = mem_heap_lo() + WSIZE;
    void *bound = mem_heap_lo() + (DSIZE * 2) * 8;
    while (list_start < bound) {
        print_single_list(list_start);
        printf("==================\n");
        list_start += (DSIZE * 2);
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
    //printf("malloc size is %d\n", size);
    if (size == 0) {
        return NULL;
    }

    size_t newsize = MAX(ALIGN(size + WSIZE * 2), MINIMUM_SIZE);

    void *ptr = find_fit(newsize);

    delete_list(ptr + WSIZE);
    size_t oldsize = GET_SIZE(ptr);
    int delta_size = oldsize - newsize;
    if (delta_size < MINIMUM_SIZE) {
        newsize = oldsize;
    }
    PUT(ptr, PACK(newsize, 1));
    void *footer = (void *)((char *)ptr + newsize - WSIZE); // void ptr arithmetic undefined
    PUT(footer, PACK(newsize, 1));
    
    if (delta_size >= MINIMUM_SIZE) {
        // pay attention to the equal sign or it will cause bugs!
        // splitting!
        PUT(footer + WSIZE, PACK(delta_size, 0));
        PUT(footer + delta_size, PACK(delta_size, 0));
        append_list(footer + WSIZE * 2);
    }
    // printf("malloc : %d\n", size);
    /*
    printf("malloc : %d\n", size);
    print_list();
    print_heap();
    */
    return (void *) ((char *)ptr + WSIZE);
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

    void *prev_footer = ptr - WSIZE * 2;
    int prev_alloc = GET_ALLOC(prev_footer);

    void *next_header = footer + WSIZE;
    int next_alloc = GET_ALLOC(next_header);

    if (prev_alloc && next_alloc) {
        PUT(header, PACK(size, 0));
        PUT(footer, PACK(size, 0));
        append_list(ptr);
        return header;
    }

    if (prev_alloc && !next_alloc) {
        void *next_footer = next_header + GET_SIZE(next_header) - WSIZE;
        int new_size = size + GET_SIZE(next_header);
        PUT(header, PACK(new_size, 0));
        PUT(next_footer, PACK(new_size, 0));
        delete_list(next_header + WSIZE);
        append_list(ptr);
        return header;
    }

    if (!prev_alloc && next_alloc) {
        void *prev_header = prev_footer - GET_SIZE(prev_footer) + WSIZE;
        int new_size = size + GET_SIZE(prev_footer);
        PUT(footer, PACK(new_size, 0));
        PUT(prev_header, PACK(new_size, 0));
        delete_list(prev_header + WSIZE);
        append_list(prev_header + WSIZE);
        return prev_header;
    }

    if (!prev_alloc && !next_alloc) {
        void *prev_header = prev_footer - GET_SIZE(prev_footer) + WSIZE;
        void *next_footer = next_header + GET_SIZE(next_header) - WSIZE;
        int new_size = size + GET_SIZE(next_header) + GET_SIZE(prev_footer);
        PUT(prev_header, PACK(new_size, 0));
        PUT(next_footer, PACK(new_size, 0));
        delete_list(prev_header + WSIZE);
        delete_list(next_header + WSIZE);
        append_list(prev_header + WSIZE);
        return prev_header;
    }
    return NULL;
    
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    //printf("free %p\n", ptr);
    coalesce(ptr);

    
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
    
    if (ptr == NULL) {
        return mm_malloc(size);
    }
    if (size == 0) {
        mm_free(ptr);
        return NULL;
    }
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;

    copySize = GET_SIZE((char *)(oldptr - WSIZE));
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
















