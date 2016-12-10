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

int size_of_heap = 1000;

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "Innocent Bystanders",
    /* First member's full name */
    "Muhamed Rahman",
    /* First member's github username*/
    "mrahman13",
    /* Second member's full name (leave blank if none) */
    "Erica Chio",
    /* Second member's github username (leave blank if none) */
    "ericachio"
    /* Third member's full name (leave blank if none) */
    "Jason Gross"
    /* Third member's github username (leave blank if none) */
    "anonymouse123e"
};
size_t* heap;
/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    //Find large area of available space. If enough, set equal to size_of_heap. If not, return -1.
    heap = malloc(size_of_heap); //[size_of_heap];
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    /*int newsize = ALIGN(size + SIZE_T_SIZE);
    void *p = mem_sbrk(newsize);
    if (p == (void *)-1)
    return NULL;
    else {
        *(size_t *)p = size;
        return (void *)((char *)p + SIZE_T_SIZE);
    }*/
    int footer;
    int header = 0;
    while(header < size_of_heap){
        footer = 0;//header + heap[header]/8 + 1;
        if(heap[header]%8 == 0 && (heap[header]/8) >= size){//if heap is free and large enough
            heap[header] = size + 1;//mark header as allocated, set to new size
            //printf("%s\t", "heap[header]:");
            //printf("%zu\t", heap[header]);
            heap[header + size/8 + 1] = heap[header];//create new footer, same as header
            //printf("%s\t", "heap[footer]:");
            //printf("%zu\n", heap[footer]);
            heap[header + size/8 + 2] = heap[footer] - size - 16;//create new header after new footer
            heap[footer] = heap[header + size/8 + 2];//set old footer to equal new header
            //printf("%s\t", "heap[header+1]:");
            //printf("%lu\n", (heap[header+1]) + 8);
            size_of_heap++;
            return (void*)(heap[header + 1] + 8);//return pointer to first block
            //[80][ ][ ][ ][ ][ ][80] -> [33][ ][ ][33][16][ ][16], for example
        }
        else{
            header += heap[header]/8 + 2;
        }
    }
    if (header == size_of_heap - 1){
        heap[header] = (size_t) mem_sbrk(size_of_heap);
        size_of_heap = size_of_heap  * 2; 
    }
    return (void*)-1;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    size_t* header = (size_t*)ptr - 1;
    size_t* footer = header + *header/8 + 1;
    if(*header %8 == 1)//if allocated
        *header--;
        *footer--;
    if (*(header-1) %8 == 0)//If previous block is free
        header = header - *(header-1)/8 - 2;//move header to the header of the previous block
        *header += *footer + 16;//Increment the header's size val to include both blocks and consumed footer/header.
        *footer = *header;//make footer match
    if (*(footer+1) %8 == 0)//If next block is free
        footer = footer + *(footer+1)/8 + 2;//move footer to the footer of the next block
        *header += *footer + 16;
        *footer = *header;    
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    /*void *oldptr = ptr;
    void *newptr;
    size_t copySize;
    
    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;
    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;*/
    if (ptr == NULL)
    {
        mm_malloc(size);
    }else if (size == 0)
    {
        mm_free(ptr);
    }

}




void mm_checkheap(int verbose) 
{
    return;
}











