CC=gcc
CFLAGS=-O3 -fPIC -ggdb3
DEPS=my_malloc.h
MALLOC_VERSION=BF
WDIR=/Users/JohnDong/Documents/Study/Graduate/Master\ in\ DUKE/ECE650/hw1/code/

all: equal_size_allocs small_range_rand_allocs large_range_rand_allocs

equal_size_allocs: equal_size_allocs.c
	$(CC) $(CFLAGS) -I$(WDIR) -L$(WDIR) -D$(MALLOC_VERSION) -Wl,-rpath,$(WDIR) -o $@ equal_size_allocs.c -lmymalloc

small_range_rand_allocs: small_range_rand_allocs.c
	$(CC) $(CFLAGS) -I$(WDIR) -L$(WDIR) -D$(MALLOC_VERSION) -Wl,-rpath,$(WDIR) -o $@ small_range_rand_allocs.c -lmymalloc

large_range_rand_allocs: large_range_rand_allocs.c
	$(CC) $(CFLAGS) -I$(WDIR) -L$(WDIR) -D$(MALLOC_VERSION) -Wl,-rpath,$(WDIR) -o $@ large_range_rand_allocs.c -lmymalloc

clean:
	rm -f *~ *.o equal_size_allocs small_range_rand_allocs large_range_rand_allocs

clobber:
	rm -f *~ *.o

