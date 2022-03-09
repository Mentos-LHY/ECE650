#ifndef MY_MALLOC_H
#define MY_MALLOC_H

#define node_size sizeof(BlockNode)

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

//Data structure to hold the free list
typedef struct memBlockNode {
  size_t size;
  struct memBlockNode * next;
  struct memBlockNode * prev;
} BlockNode;

//First Fit
void * ff_malloc(size_t size);
void ff_free(void * ptr);

//Best Fist
void * bf_malloc(size_t size);
void bf_free(void * ptr);

//check available block on free list by ff
void * ff_checkBlock(size_t size);
//check available block on free list by bf
void * bf_checkBlock(size_t size);

//Get new block using sbrk() when checking free list fail
void * my_sbrk(size_t size);

//free the block and insert (back) into the free list
void freeBlock(void * block);

//Merge 2 adjacent free blocks
void mergeBlock(BlockNode * block);

//Split the block of requested size from a larger free block and remove it from the free list
void * splitBlock(BlockNode * block, size_t size);

//for performance test
unsigned long get_data_segment_size();

unsigned long get_data_segment_free_space_size();

#endif
