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

#define DSIZE 8

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

void print_heap(){
    int i;
    for(i=0; i<mem_heapsize()/DSIZE; i++){
        printf("[%zx]", heap[i]);
    }
    printf("\n");
}

size_t* coalesce(size_t* header){
    size_t* footer = header + (*header)/DSIZE + 1;
    //printf("%d\n", *header % DSIZE);
    if(*header % DSIZE == 0){
        if (*(header-1) % DSIZE == 0){//If previous block is free
            //printf("a");
            header = header - *(header-1)/DSIZE - 2;//move header to the header of the previous block
            *header += *footer + 2*DSIZE;//Increment the header's size val to include both blocks and consumed footer/header.
            *footer = *header;//make footer match
        }
        if (*(footer+1) % DSIZE == 0){//If next block is free
            //printf("b");
            footer = footer + *(footer+1)/DSIZE + 2;//move footer to the footer of the next block
            *header += *footer + 2*DSIZE;
            *footer = *header;
        }
    }
    //printf("%zx\n", header);
    return header;
}

size_t* extend_heap(size_t dwords){//helper function to extend heap.
    if(dwords%2 == 1)
        dwords++;//align to 16 bytes
    //printf("Extending the heap by %zx bytes.\n", dwords);
    size_t* next = (size_t*)mem_sbrk(dwords*DSIZE + 2*DSIZE);
    if(next == (size_t*)-1)
        return (size_t*)-1;
    //[1][1][33][ ][ ][33][1] -> [1][1][33][ ][ ][33][1][n][ ][ ][ ] for dwords = 2
    //modify the last dwords + 3 entries in the heap. Set last one to epilogue, first one to header,
    //second-to-last to footer.
    *(next+dwords+1) = 1;//set last item to epilogue.
    *(next-1) = dwords*DSIZE;//set previous epilogue header to new header, not allocated.
    *(next+dwords) = *(next-1);//set new footer equal to new header.
    int i;
    for(i=0; i<dwords; i++){
        *(next+i) = 0;
    }
    return coalesce(next-1);
}

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    heap = (size_t*)mem_sbrk(4*DSIZE);
    if(heap == (size_t*)-1)
        return -1;
    heap[0] = 0;//First byte is 0
    heap[1] = 1;//Prologue header = 0 size, allocated
    heap[2] = 1;//Prologue footer = 0 size, allocated
    heap[3] = 1;//Epilogue header = 0 size, allocated
    print_heap();
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    //printf("Doing the malloc: %zx\n", size);
    size_t asize = (size-1)/DSIZE + 1;//Calculate the number of array entries we need.
    if(asize % 2 == 1)//Make sure it's 16-byte aligned.
        asize++;
    size_t footer = 0;
    size_t header = 0;
    size_t header2 = 0;
    size_t footer2 = 0;
    while(header < mem_heapsize()/DSIZE){
        footer = header + heap[header]/DSIZE + 1;
        //printf("%zu\n", heap[header]%DSIZE);
        if(heap[header]%DSIZE == 0){//if block is free
            if(heap[header] == size*DSIZE){//and exactly the right size
                heap[header]++;
                heap[footer]++;
                //printf("Whoa! Lucky!\n");
                return (void*)(heap + header + 1);//return pointer to first block
            }
            else if(heap[header] >= (asize+4)*DSIZE){//or large enough
                //printf("Okay. Cool.\n");
                //print_heap();
                //printf("%zx\n", heap[footer]);
                header2 = header + asize + 2;
                footer2 = footer;
                footer = header + asize + 1;//to eliminate conufusion, these are the locations of the headers and footers.
                //line above was head + size/DSIZE s1 before
                heap[header] = asize*DSIZE + 1;//block 1 is size bytes, allocated
                heap[footer] = heap[header];//footer = header
                heap[footer2] = heap[footer2] - (asize+2)*DSIZE;//shrink second block by the size of block 1, and then make room for new header+footer
                //printf("%zx\n", heap[footer2]);
                heap[header2] = heap[footer2];//footer2 = header2
                //print_heap();
                return (void*)(heap + header + 1);//return pointer to first block
                //[80][ ][ ][ ][ ][ ][80] -> [33][ ][ ][33][16][ ][16], for example
            }
        }
        header = footer + 1;
    }
    size_t * loc = extend_heap(asize);// this was at first * size_t but compiler didnt read that right
    //printf("%zx\n", *(size_t*)0x80c29a8ed028);
    //printf("Extending heap\n");
    //printf("%zx\n", size);
    if(loc == (size_t*)-1){
        return (void*)-1;
    }
    else{
        *loc = *loc + 1;//mark header as allocated
        *(loc + (*loc)/DSIZE +1) = *(loc + (*loc)/DSIZE +1) + 1;//mark footer as allocated
    }
    //print_heap();
    return (void*)(loc + 1);
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    //printf("%s\n", "FREE AT LAST\n");
    size_t* header = (size_t*)ptr - 1;
    size_t* footer = header + *header/8 + 1;
    if(*header %8 == 1){//if allocated
        *header = *header-1;//deallocate
        *footer = *footer-1;
    }
    //print_heap();
    coalesce(header);
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
    printf("REALLOC");
    if (ptr == NULL)
    {
        mm_malloc(size);
        return (void*)-1;
    }else if (size == 0)
    {
        mm_free(ptr);
        return (void*)-1;
    }else{
        return (void*)-1;
    }

}




void mm_checkheap(int verbose) 
{
    return;
}
