#ifndef __CACHE_ALLOC_H__
#define __CACHE_ALLOC_H__

struct node
{
	struct node* next;
};

struct cache_allocer{
	unsigned char* cache_mem;
	int capacity;
	int elem_size;
	struct node* free_list;
};

struct cache_allocer* create_cache_alloc(int capacity,int elem_size);

void destroy_cache_allocer(struct cache_allocer* allocer);

void* cache_alloc(struct cache_allocer* allocer,int elem_size);

void cache_free(struct cache_allocer* allocer,void* mem);

#endif