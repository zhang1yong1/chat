#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <assert.h>
#include <sys/stat.h>
#include "socket_server.h"
#include "threadpool.h"
#include "friend_chat.h"
#include "cache_alloc.h"

#define SOCKET_MESSAGE_CAPACITY  1000
static struct cache_allocer* sm_allocer = NULL;

#define MY_MALLOC(elem_size) cache_alloc(sm_allocer,elem_size)
#define MY_FREE(elem)   cache_free(sm_allocer,(void*)elem)

struct thread_pool_t* p = NULL;
struct socket_server* ss = NULL;

void*
run(void* data){
	struct socket_server* ss = (struct socket_server*) data;

	for(;;){
		struct socket_message* result = (struct socket_message*)MY_MALLOC(sizeof(struct socket_message)) ; //result main文件需要打印结构体内部内容,所以结构体定义必须include,必须放在socket_server.h中
		int r = socket_server_poll(ss,result);
		if(r == SOCK_DATA){
			printf("%d:recv----:%s\n",result->c_fd,(char*)result->buff);
			//socket_server_send(ss,result);
			friend_chat_server_loop(result);			
			continue;
		}
		if(r == SOCK_SEND){
			free(result->buff);
			MY_FREE(result);
			continue;
		}
		if(r == SOCK_ERROR){
			printf("socket server poll error\n");
			free(result->buff);
			MY_FREE(result);
			continue;
		}
		if(r == 0){
			free(result->buff);
			MY_FREE(result);
			continue;
		}
	}
}

void 
init_firefly_allocer(){
	if (sm_allocer == NULL)
	{
		sm_allocer = create_cache_alloc(SOCKET_MESSAGE_CAPACITY,sizeof(struct socket_message));
	}
}

int main(int argc, char const *argv[]){
	printf("firefly start....\n");
	init_firefly_allocer();
	p = thread_pool_create(1,10,10);
	pthread_t tid;
	//pthread_create(&pid,0,run,0);
	ss = socket_server_create();
	if(ss == NULL){
		goto firefly_failed;
	}
	printf("socket create....\n");
	int r;
	r = socket_server_bind(ss,8888);
	if(r == -1){
		goto firefly_failed;
	}
	printf("socket bind....8888\n");

	r = socket_server_listen(ss);
	if(r == -1){
		goto firefly_failed;
	}
	printf("socket listen....\n");

	//新建一个线程来控制
	pthread_create(&tid,0,run,(void*)ss);
	pthread_join(tid,(void**)0);

firefly_failed:
	socket_server_exit(ss);
	thread_pool_free(p);
	printf("firefly stop....\n");
	exit(-1);
	return 0;
}