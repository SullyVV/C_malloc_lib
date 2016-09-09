#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <unistd.h>
#include <stdbool.h>
typedef struct meta_info* p_meta;
struct meta_info{
  int free;   //0:free, 1:busy
  int size;
  p_meta prev;
  p_meta next;
  char data[1];
};
size_t META_INFO = 2*sizeof(int) + 2*sizeof(p_meta);   //data field is the start of data stored in the block
void* start = NULL;
void* last = NULL;
/*algotirhm
aligh the size so its multiple of 4
check if the heap is allocated
    if yes, search among the allocated blocks to see if there is fitting block
          if yes, change the meta-info of the block and return a ptr to that block
	  otherwise, allocated a new block on heap and return the pointer
    otherwise, allocate a new block on heap and return the ptr
*/
size_t align(size_t size){
  if(size%4 == 0){
    return size;
  }else{
    return (size-size%4+4);
  }
}
void divideBlock(p_meta curr, size_t size){
  p_meta n;
  n = (p_meta)(curr->data + size);
  n->next = curr->next;
  n->prev = curr;
  n->size = curr->size - size - META_INFO;
  n->free = 0;
  curr->next = n;
}

p_meta extend_heap(p_meta last, size_t size){
  p_meta curr;
  curr = sbrk(0);
  if(sbrk(size+META_INFO) == (void*) -1){
    return NULL;
  }
  if(last){
    last->next = curr;
  }
  curr->next = NULL;
  curr->prev = last;
  curr->size = size;
  curr->free = 1;
  return curr;
}
/*
p_meta find_block(p_meta start, size_t size){
  p_meta curr = start;
  while(curr->size < size || curr->free == 1){
    curr = curr->next;
    if(curr == NULL){
      return NULL;
    }
  }
  if(curr->size > size + META_INFO){
    divideBlock(curr, size);
  }
  curr->size = size;
  curr->free = 1;
  return curr;
}
*/
//no need to do division within one block
p_meta find_block(p_meta start, size_t size){
  p_meta curr = start;
  while(curr->size < size || curr->free == 1){
    curr = curr->next;
    if(curr == NULL){
      return NULL;
    }
  }
  curr->free = 1;
  return curr;
}

p_meta getMeta(void* curr){
  char* data = curr;
  return (p_meta)(data-META_INFO);
}
bool checkPtr(void* curr){
  if(start){
    if(curr>start && curr < sbrk(0)){
      p_meta block = getMeta(curr);
      if(block->free == 1){
	return true;
      }
    }
  }
  return false;
}

p_meta mergeBlock(p_meta curr){
  if(curr->next && (curr->next->free == 0)){
    curr->size = curr->size + META_INFO + curr->next->size;
    curr->next = curr->next->next;
    curr->next->prev = curr;
  }
  return curr;
}

void* ff_malloc(size_t size){
  size_t newsize  = align(size);
  p_meta curr = NULL;
  if(start){
    curr = find_block(start, newsize);
    if(!curr){
      curr = extend_heap(last,newsize);
      last = curr;
    }
  }else{
    curr = extend_heap(last, newsize);
    start = curr;
    last = start;
  }
  return curr->data;
}


/*algorithm for best fit:
align the size first
diff = 0, tmp = NULL
check if base is NULL
      if base is NULL, do extendheap
otherwise
      iterate through all blocks in the heap
        if curr->size >= size
          difference = curr->size - size
	  tmp = curr
      if tmp!= null
         return tmp->data
      otherwise
         fo extendheap
*/

void* bf_malloc(size_t size){
  size_t newsize = align(size);
  p_meta tmp = NULL;
  p_meta curr = NULL;
  size_t diff = INT_MAX;
  if(start){
    curr = start;
    while(curr){
      if(curr->free == 0 && curr->size >= newsize){
	if((curr->size-newsize) < diff){
	  tmp = curr;
	  diff = curr->size - newsize;
	}
      }
      curr = curr->next;
    }
    if(tmp){
      /*
      //this part is for block division within a block, but we dont need to do this
      if(tmp->size > META_INFO+newsize){
	divideBlock(tmp, newsize);
      }
      tmp->free = 1;
      tmp->size = newsize;
      return tmp->data;
      */
      tmp->free = 1;
      return tmp->data;
    }else{
      curr = extend_heap(last, newsize);
      last = curr;
    }
  }else{
    curr = extend_heap(last, newsize);
    start = curr;
    last = curr;
  }
  return curr->data;
}

void* wf_malloc(size_t size){
  size_t newsize = align(size);
  p_meta tmp = NULL;
  p_meta curr = NULL;
  size_t diff = 0;
  if(start){
    curr = start;
    while(curr){
      if(curr->free == 0 && curr->size >= newsize){
	if((curr->size-newsize) > diff){
	  tmp = curr;
	  diff = curr->size - newsize;
	}
      }
      curr = curr->next;
    }
    if(tmp){
      /*
      //this part is for block division within a block, but we dont need to do this
      if(tmp->size > META_INFO+newsize){
	divideBlock(tmp, newsize);
      }
      tmp->free = 1;
      tmp->size = newsize;
      return tmp->data;
      */
      tmp->free = 1;
      return tmp->data;
    }else{
      curr = extend_heap(last, newsize);
      last = curr;
    }
  }else{
    curr = extend_heap(last, newsize);
    start = curr;
    last = curr;
  }
  return curr->data;
}


/*free algorithm
check the address to see if it is malloced
     if yes, get to the block head
         merge the block with the previous block
  	    check if curr has next block
	       if yes, merge curr with next blcok 
	    otherwise, check if curr has prev
	               if yes, curr->prev->next = NULL, change the program break
		       otherwise, set base to NULL
*/
void myfree(void* ptr){
  if(checkPtr(ptr)){
    p_meta curr = getMeta(ptr);
    curr->free = 0;
    if(curr->prev && (curr->prev->free == 0)){
      curr = mergeBlock(curr->prev);
    }
    if(curr->next){
      curr = mergeBlock(curr);
    }
    /*
      //this part is for garbage collection, we dont need to do this
    else{
      if(curr->prev){
	last = curr->prev;
	curr->prev->next = NULL;
	brk(curr);
      }else{
	start = NULL;
      }
    }
    */
  }else{
    printf("This is an invalid pointer to free\n");
  }
}

void ff_free(void* ptr){
  if(checkPtr(ptr)){
    p_meta curr = getMeta(ptr);
    curr->free = 0;
    if(curr->prev && (curr->prev->free == 0)){
      curr = mergeBlock(curr->prev);
    }
    if(curr->next){
      curr = mergeBlock(curr);
    }
    /*
      //this part is for garbage collection, we dont need to do this
    else{
      if(curr->prev){
	last = curr->prev;
	curr->prev->next = NULL;
	brk(curr);
      }else{
	start = NULL;
      }
    }
    */
  }else{
    printf("This is an invalid pointer to free\n");
  }
}

void bf_free(void* ptr){
  if(checkPtr(ptr)){
    p_meta curr = getMeta(ptr);
    curr->free = 0;
    if(curr->prev && (curr->prev->free == 0)){
      curr = mergeBlock(curr->prev);
    }
    if(curr->next){
      curr = mergeBlock(curr);
    }
    /*
      //this part is for garbage collection, we dont need to do this
    else{
      if(curr->prev){
	last = curr->prev;
	curr->prev->next = NULL;
	brk(curr);
      }else{
	start = NULL;
      }
    }
    */
  }else{
    printf("This is an invalid pointer to free\n");
  }
}

void wf_free(void* ptr){
  if(checkPtr(ptr)){
    p_meta curr = getMeta(ptr);
    curr->free = 0;
    if(curr->prev && (curr->prev->free == 0)){
      curr = mergeBlock(curr->prev);
    }
    if(curr->next){
      curr = mergeBlock(curr);
    }
    /*
      //this part is for garbage collection, we dont need to do this
    else{
      if(curr->prev){
	last = curr->prev;
	curr->prev->next = NULL;
	brk(curr);
      }else{
	start = NULL;
      }
    }
    */
  }else{
    printf("This is an invalid pointer to free\n");
  }
}

unsigned long get_data_segment_size(){
  unsigned long size = 0;
  p_meta curr = start;
  while(curr){
    size += META_INFO + curr->size;
    curr = curr->next;
  }
  return size;
}

unsigned long get_data_segment_free_space_size(){
  unsigned long free_size = 0;
  p_meta curr = start;
  while(curr){
    if(curr->free == 0){
      free_size += curr->size;
    }
    curr = curr->next;
  }
  return free_size;
}
