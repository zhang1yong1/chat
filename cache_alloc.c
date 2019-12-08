#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cache_alloc.h"


struct cache_allocer* 
create_cache_alloc(int capacity,int elem_size){
	struct cache_allocer* allocer = (struct cache_allocer*)malloc(sizeof(struct cache_allocer));	
	memset(allocer,0,sizeof(struct cache_allocer));
	elem_size = (elem_size < sizeof(struct node)) ? sizeof(struct node):elem_size;
	allocer->elem_size = elem_size;
	allocer->capacity = capacity;
	allocer->cache_mem = malloc(capacity* elem_size);//预先就把一大块内存申请好
	memset(allocer->cache_mem,0,capacity* elem_size);

	allocer->free_list = NULL;

	//申请的时候,就把每个节点之间通过链表连接起来
	for (int i = 0; i < capacity; ++i)
	{
		struct node* walk = (struct node*)(allocer->cache_mem + i*elem_size);
		walk->next = allocer->free_list;
		allocer->free_list = walk;
	}

	return allocer;
}


void 
destroy_cache_allocer(struct cache_allocer* allocer){
	if(allocer->cache_mem != NULL){
		free(allocer->cache_mem);
	}

	free(allocer);
}

void* 
cache_alloc(struct cache_allocer* allocer,int elem_size){
	if (allocer->elem_size < elem_size)
	{
		return malloc(elem_size);
	}

	if (allocer->free_list != NULL)
	{
		void* now = allocer->free_list;
		allocer->free_list = allocer->free_list->next;
		return now;
	}

	return malloc(elem_size);
}

void 
cache_free(struct cache_allocer* allocer,void* mem){
	if (((unsigned char*)mem) >= allocer->cache_mem &&
		((unsigned char*)mem) < allocer->cache_mem + allocer->capacity* allocer->elem_size
		)//内存地址在开始跟结束之间
	{
		//只需要把节点重新连接,等重新申请的时候,对应区域内存会重新memset成0
		struct node*  node = mem;
		node->next = allocer->free_list;
		allocer->free_list = node;
		return;
	}

	free(mem);
}