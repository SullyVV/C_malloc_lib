#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <assert.h>
typedef struct meta_info* p_meta;
struct meta_info;
size_t META_INFO;   //data field is the start of data stored in the block
void* start;
void* last;
//pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
size_t align(size_t size);
void divideBlock(p_meta curr, size_t size);
p_meta extend_heap(p_meta last, size_t size);
p_meta find_block(size_t size);
p_meta getMeta(void* curr);
bool checkPtr(void* curr);
//p_meta mergeBlock(p_meta curr);
void* ff_malloc(size_t size);
void* bf_malloc(size_t size);
void* wf_malloc(size_t size);
void myfree(void* ptr);
void ff_free(void* ptr);
void bf_free(void* ptr);
void wf_free(void* ptr);
unsigned long get_data_segment_size();
unsigned long get_data_segment_free_space_size();
//Thread Safe malloc/free
void *ts_malloc(size_t size);
void ts_free(void *ptr);
p_meta ts_extend_heap(size_t size);
