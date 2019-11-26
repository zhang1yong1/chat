#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <assert.h>
#include <sys/stat.h>
#include "socket_server.h"


#define MY_MALLOC malloc

void*
run(void* data){
	struct socket_server* ss = (struct socket_server*) data;
	struct socket_message* result = (struct socket_message*)MY_MALLOC(sizeof(struct socket_message)) ; //result main文件需要打印结构体内部内容,所以结构体定义必须include,必须放在socket_server.h中

	for(;;){
		int r = socket_server_poll(ss,result);
		if(r == SOCK_DATA){
			printf("%d:recv----:%s\n",result->c_fd,(char*)result->buff);
			socket_server_send(ss,result);
			continue;
		}
		if(r == SOCK_ERROR){
			printf("socket server poll error\n");
			continue;
		}
		if(r == 0){
			continue;
		}
	}
}

int main(int argc, char const *argv[]){
	printf("firefly start....\n");
	pthread_t tid;
	//pthread_create(&pid,0,run,0);
	struct socket_server* ss = socket_server_create();
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
	printf("firefly stop....\n");
	exit(-1);
	return 0;
}