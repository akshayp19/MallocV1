/*
 We used the textbook to understand how to do it, but we utilized our own code.
 
 Our heap is implemented as an array of size_ts, so that each entry in the array is 8 bytes.
 Headers and footers are also single entries in the array. Additionally, since the heap must be
 16-byte aligned, blocks of memory exist as header/footer-capped sections of the array with length
 divisible by two. Headers and footers store size values equivalent only to the number of bytes
 betweeen them, such that a header and a footer right next to each other in memory would both have size
 values of 0.

 The heap begins as an array of size four, containing a blank entry, a prologue block, and an
 epilogue header. New blocks are added only when required by malloc to utilize memory as efficiently
 as possible. Malloc searches through every block in the array and, if it is free, checks the size.
 Blocks within ~4 array items of the correct size are returned as-is. Larger blocks, which can be split
 and have non-empty remainder blocks, are accordingly split to the exact size and returned. Free and
 allocated blocks are right next to each other in memory - due to coalescing, every free block will be
 surrounded on both sides by allocated blocks, but allocated blocks may have either free or allocated
 blocks next to them.

 Free simply checks to ensure that the block has been allocated, and then removes the allocated flags
 on it. Then, it coalesces.

 Realloc takes care of the boundary cases first, and then if neither ptr is NULL nore size is 0, it
 will immediately free the area in memory. The area will automatically coalesce, and so if it is of the
 correct size after coalescing, we will return the pointer to the same block's new header. Otherwise,
 we will use malloc to find a proper-sized area.
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
    "ericachio",
    /* Third member's full name (leave blank if none) */
    "Jason Gross",
    /* Third member's github username (leave blank if none) */
    "anonymouse123e"
};
size_t* heap;

//Coalesce is a helper function for when we need to merge free blocks.
size_t* coalesce(size_t* header){
    size_t* footer = header + (*header)/DSIZE + 1;//Find the footer.
    if(*header % DSIZE == 0){//If the current block is free,
        if (*(header-1) % DSIZE == 0){//and previous block is free
            header = header - *(header-1)/DSIZE - 2;//move header to the header of the previous block
            *header += *footer + 2*DSIZE;//Increment the header's size val to include both blocks and consumed footer/header.
            *footer = *header;//make footer match
        }
        if (*(footer+1) % DSIZE == 0){//If next block is free
            footer = footer + *(footer+1)/DSIZE + 2;//move footer to the footer of the next block
            *header += *footer + 2*DSIZE;//Increment the header to include both blocks and consumed header/footer
            *footer = *header;//make the footer match
        }
    }
    return header;//this should be a pointer to the header of the coalesced block
}

size_t* extend_heap(size_t dwords){//helper function to extend heap.
    if(dwords%2 == 1)
        dwords++;//align to 16 bytes
    //printf("Extending the heap by %zx bytes.\n", dwords);
    size_t* next = (size_t*)mem_sbrk(dwords*DSIZE + 2*DSIZE);//Extend the heap.
    if(next == (size_t*)-1)//If the extension failed, return -1.
        return (size_t*)-1;
    *(next+dwords+1) = 1;//set last item to epilogue.
    *(next-1) = dwords*DSIZE;//set previous epilogue header to new header, not allocated.
    *(next+dwords) = *(next-1);//set new footer equal to new header.

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
    //print_heap();
    return 0;
}

/* 
 * mm_malloc - Search through the heap. Return the first block of the appropriate size. 
 * If the block is almost exactly the right size, just return it as-is. Otherwise, if it's
 * considerably larger, split it and then return a perfectly-sized block.
 */
void *mm_malloc(size_t size)
{
    //printf("Doing the malloc: %zx\n", size);
    size_t asize = (size-1)/DSIZE + 1;//Calculate the number of array entries we need.
    if(asize % 2 == 1)//Make sure it's 16-byte aligned.
        asize++;
    size_t header = 3;//Start after the prologue, set headers and footers.
    size_t footer = header + heap[header]/DSIZE + 1;
    size_t header2 = 0;//these are for if we have to split the block.
    size_t footer2 = 0;
    while(header < mem_heapsize()/DSIZE){
        //printf("%zu\n", heap[header]%DSIZE);
        footer = header + heap[header]/DSIZE + 1;
        if(heap[header]%DSIZE == 0){//if block is free
            if(heap[header] >= asize*DSIZE && heap[header] < (asize+4)*DSIZE){//and exactly the right size
                heap[header]++;
                heap[footer]++;
                return (void*)(heap + header + 1);//return pointer to first block
            }
            else if(heap[header] >= (asize+4)*DSIZE){//or large enough

                header2 = header + asize + 2;
                footer2 = footer;
                footer = header + asize + 1;//to eliminate conufusion, these are the locations of the headers and footers.
                //line above was head + size/DSIZE s1 before
                heap[header] = asize*DSIZE + 1;//block 1 is size bytes, allocated
                heap[footer] = heap[header];//footer = header
                heap[footer2] = heap[footer2] - (asize+2)*DSIZE;//shrink second block by the size of block 1, and then make room for new header+footer

                heap[header2] = heap[footer2];//footer2 = header2
                return (void*)(heap + header + 1);//return pointer to first block
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
    size_t* header = (size_t*)ptr - 1;
    //printf("FREE AT LAST\n");
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
    if (ptr == NULL)
    {
        return mm_malloc(size);
    }else{
        mm_free(ptr);
        if(!size) return(void*)-1;
        size_t* header = (size_t*)ptr - 1;
        void* newptr = (void*)(header + (*header)/DSIZE - *(header + (*header)/DSIZE + 1)/DSIZE);
        header = (size_t*)newptr;
        //Because of how  we implemented free, the above code will always give us the header of the new block's header after coalescing.
        if(*(size_t*)newptr >= size){
            newptr+=8;//Make newptr point to the block instead of its header.
            (*header)++;
            (*(header + (*header)/DSIZE + 1))++;
        }else{
            newptr = mm_malloc(size);
        }
        return memcpy(newptr, ptr, *(size_t*)(ptr-8));
    }
}




void mm_checkheap(int verbose) 
{
    return;
}