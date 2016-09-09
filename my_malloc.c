#include "my_malloc.h"
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
  if(curr->next){
    curr->next->prev = n;
  }else{
    last = n;
  }
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

p_meta getMinBlock(p_meta curr, size_t newsize){
  p_meta tmp = NULL;
  size_t diff = INT_MAX;
  while(curr){
    if(curr->free == 0 && curr->size >= newsize){
      if((curr->size - newsize) < diff){
	tmp = curr;
	diff = curr->size - newsize;
      }
    }
    curr = curr->next;
  }
  return tmp;
}

p_meta find_block(size_t size){
  p_meta curr = start;
  while(curr->size < size || curr->free == 1){
    curr = curr->next;
    if(curr == NULL){
      return NULL;
    }
  }
  if(curr->size > size + META_INFO){
    divideBlock(curr, size);
    curr->size = size;
  }
  curr->free = 1;
  return curr;
}

p_meta getMeta(void* curr){
  char* data = curr;
  return (p_meta)(data-META_INFO);
}
bool checkPtr(void* curr){
  if(curr){
    if(start){
      if(curr>start && curr < sbrk(0)){
	p_meta block = getMeta(curr);
	if(block->free == 1){
	  return true;
	}
      }
    }
  }
  return false;
}

p_meta mergeBlock(p_meta curr){
  if(curr&&curr->next){
    if(curr->next->free == 0){
      curr->size = curr->size + META_INFO + curr->next->size;
      curr->next = curr->next->next;
      if(curr->next){
	curr->next->prev = curr;
      }
    }
  }
  return curr;
}

void* ff_malloc(size_t size){
  size_t newsize  = align(size);
  p_meta curr = NULL;
  if(start){
    curr = find_block(newsize);
    if(!curr){
      curr = extend_heap(last,newsize);
      last = curr;
    }
  }else{
    curr = extend_heap(last, newsize);
    start = curr;
    last = curr;
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
      
      if(tmp->size > META_INFO+newsize){
	divideBlock(tmp, newsize);
	tmp->size = newsize;
      }
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
      
        if(tmp->size > META_INFO+newsize){
	  divideBlock(tmp, newsize);
	  tmp->size = newsize;
      }
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
    if(curr->prev){
      if(curr->prev->free == 0){
	curr = mergeBlock(curr->prev);
      }
    }
    if(curr->next){
      mergeBlock(curr);
    }
  }
}

void ff_free(void* ptr){
  myfree(ptr);
}

void bf_free(void* ptr){
  myfree(ptr);
}

void wf_free(void* ptr){
  myfree(ptr);
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
      free_size += META_INFO + curr->size;
    }
    curr = curr->next;
  }
  return free_size;
}

/*
 void* ts_malloc(size_t size){
 size_t newsize = align(size);
 p_meta tmp = NULL;
 p_meta curr = NULL;
 size_t diff = INT_MAX;
 //pthread_mutex_lock(&mut);
 if(start){
 tmp = getMinBlock(start, newsize);
 //this is a critical section
 pthread_mutex_lock(&mut);
 if(tmp && tmp->free==0){
 //if conditions still stands, do update, otherwise,research
 tmp->free = 1;
 
 if(tmp->size > META_INFO+newsize){
	divideBlock(tmp, newsize);
 }
 curr = tmp;
 }
 //
 else{
 //this is a critical section
 
 curr = extend_heap(last,newsize);
 last = curr;
 
 //
 }
 pthread_mutex_unlock(&mut);
 }
 else{
 //this is a critical section
 pthread_mutex_lock(&mut);
 curr = extend_heap(last,newsize);
 if(!start){
 start = curr;
 }
 last = curr;
 pthread_mutex_unlock(&mut);
 //
 }
 //pthread_mutex_unlock(&mut);
 return curr->data;
 }
 
 void ts_free(void* ptr){
 pthread_mutex_lock(&mut);
 if(checkPtr(ptr)){
 p_meta curr = getMeta(ptr);
 curr->free = 0;
 //critical section
 //pthread_mutex_lock(&mut);
 if(curr->prev && (curr->prev->free == 0)){
 curr = mergeBlock(curr->prev);
 }
 if(curr->next){
 curr = mergeBlock(curr);
 }
 //pthread_mutex_unlock(&mut);
 //
 }
 pthread_mutex_unlock(&mut);
 }
*/
