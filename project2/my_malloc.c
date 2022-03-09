#include "my_malloc.h"

//head and tail of the free list
BlockNode * head = NULL;
BlockNode * tail = NULL;
__thread BlockNode * ts_head = NULL;
__thread BlockNode * ts_tail = NULL;

//for performance test
size_t data_segment_size = 0;
unsigned long get_data_segment_size() {
  return (unsigned long)data_segment_size;
}

unsigned long get_data_segment_free_space_size() {
  size_t free_size = 0;
  BlockNode * curr = head;
  while (curr != NULL) {
    free_size += curr->size + node_size;
    curr = curr->next;
  }
  return (unsigned long)free_size;
}

/////////////////////////////////
//pthread_mutex_t my_lock;
/////////////////////////////////

void * my_sbrk(size_t size) {
  //point at the current program break
  BlockNode * new = sbrk(0);
  void * ptr = sbrk(size + sizeof(BlockNode));
  //if sbrk fail, return NULL
  if (ptr == (void *)-1) {
    return NULL;
  }
  new->size = size;
  new->next = NULL;
  new->prev = NULL;
  //only sbrk() will increase data segment size
  data_segment_size += size + node_size;
  return (void *)new + node_size;
}

void * my_sbrk_nl(size_t size) {
  pthread_mutex_lock(&my_lock);
  BlockNode * new = sbrk(0);
  void * ptr = sbrk(size + node_size);
  pthread_mutex_unlock(&my_lock);
  //if sbrk fail, return NULL
  if (ptr == (void *)-1) {
    return NULL;
  }
  new->size = size;
  new->next = NULL;
  new->prev = NULL;
  //only sbrk() will increase data segment size
  data_segment_size += size + node_size;
  return (void *)new + node_size;
}

void * ff_checkBlock(size_t size) {
  BlockNode * new = head;
  while (new != NULL) {
    if (new->size >= size) {
      return new;
    }
    else {
      new = new->next;
    }
  }
  return NULL;
}

void * bf_checkBlock(size_t size, BlockNode * head) {
  BlockNode * best = NULL;
  BlockNode * curr = head;
  while (curr != NULL) {
    if (curr->size == size) {
      best = curr;
      break;
    }
    else if (curr->size > size) {
      if (best == NULL) {
        best = curr;
        curr = curr->next;
      }
      else if (curr->size < best->size) {
        best = curr;
        curr = curr->next;
      }
      else {
        curr = curr->next;
      }
    }
    else {
      curr = curr->next;
    }
  }
  return best;  //best can be NULL
}
void * splitBlock(BlockNode * block, size_t size, BlockNode ** head, BlockNode ** tail) {
  //if remaining size cannot be tracked, just remove from the free list
  if (block->size <= (size + node_size)) {
    if (block->prev != NULL) {
      block->prev->next = block->next;
    }
    else {
      *head = block->next;
    }
    if (block->next != NULL) {
      block->next->prev = block->prev;
    }
    else {
      *tail = block->prev;
    }
    block->next = NULL;
    block->prev = NULL;
    return block;
  }
  //cut off a new one from original block in the free list
  else {
    block->size = block->size - size - node_size;
    BlockNode * new = (BlockNode *)((void *)block + node_size + block->size);
    new->size = size;
    new->next = NULL;
    new->prev = NULL;
    return new;
  }
}

/*
void * ff_malloc(size_t size) {
  if (size <= 0) {
    return NULL;
  }
  BlockNode * ptr = (BlockNode *)ff_checkBlock(size);
  if (ptr == NULL) {
    BlockNode * ans = (BlockNode *)my_sbrk(size);
    return ans;
  }
  else {
    BlockNode * ans = (BlockNode *)splitBlock(ptr, size);
    return ans + 1;
  }
}
*/

void * bf_malloc(size_t size, BlockNode ** head, BlockNode ** tail, int is_lock) {
  if (size <= 0) {
    return NULL;
  }
  BlockNode * ans = NULL;
  BlockNode * ptr = (BlockNode *)bf_checkBlock(size, *head);
  if (ptr == NULL) {
    if (is_lock == 1) {
      ans = (BlockNode *)my_sbrk(size);
    }
    else {
      ans = (BlockNode *)my_sbrk_nl(size);
    }
    return ans;
  }
  else {
    ans = (BlockNode *)splitBlock(ptr, size, head, tail);
    return ans + 1;
  }
}

void mergeBlock(BlockNode * block, BlockNode ** tail) {
  if (block->next != NULL &&
      (void *)block + node_size + block->size == (void *)block->next) {
    block->size += block->next->size + node_size;
    block->next = block->next->next;
    if (block->next != NULL) {
      block->next->prev = block;
    }
    else {
      *tail = block;
    }
  }

  if (block->prev != NULL &&
      (void *)block->prev + node_size + block->prev->size == (void *)block) {
    block->prev->size = block->prev->size + node_size + block->size;
    block->prev->next = block->next;
    if (block->prev->next != NULL) {
      block->prev->next->prev = block->prev;
    }
    else {
      *tail = block->prev;
    }
  }
}

void freeBlock(void * block, BlockNode ** head, BlockNode ** tail) {
  if (block == NULL) {
    return;
  }
  // count in BlockNode's mem
  BlockNode * begin = (BlockNode *)(block - node_size);
  if (*head == NULL && *tail == NULL) {
    *head = begin;
    *tail = begin;
  }
  else {
    BlockNode * curr = *head;
    while (curr != NULL && curr < begin) {
      curr = curr->next;
    }
    if (curr == NULL) {
      (*tail)->next = begin;
      begin->prev = *tail;
      *tail = begin;
    }
    else if (curr == *head) {
      begin->next = *head;
      (*head)->prev = begin;
      *head = begin;
    }
    else {
      begin->next = curr;
      begin->prev = curr->prev;
      curr->prev->next = begin;
      curr->prev = begin;
    }
  }
  mergeBlock(begin, tail);
}

void ff_free(void * ptr) {
  if (ptr == NULL) {
    return;
  }
  else {
    freeBlock(ptr, &head, &tail);
  }
}
void bf_free(void * ptr) {
  if (ptr == NULL) {
    return;
  }
  else {
    freeBlock(ptr, &head, &tail);
  }
}

//
//locking version
void ts_free_lock(void * ptr) {
  pthread_mutex_lock(&my_lock);
  //
  freeBlock(ptr, &head, &tail);
  //
  pthread_mutex_unlock(&my_lock);
}

void * ts_malloc_lock(size_t size) {
  pthread_mutex_lock(&my_lock);
  void * ans = bf_malloc(size, &head, &tail, 1);
  pthread_mutex_unlock(&my_lock);
  return ans;
}

//
//non-locking version
void ts_free_nolock(void * ptr) {
  freeBlock(ptr, &ts_head, &ts_tail);
}

void * ts_malloc_nolock(size_t size) {
  void * ans = bf_malloc(size, &ts_head, &ts_tail, 0);
  return ans;
}
