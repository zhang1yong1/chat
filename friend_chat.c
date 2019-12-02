#include "friend_chat.h"
#include "socket_server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "threadpool.h"

extern struct thread_pool_t* p;
extern struct socket_server* ss;

void* 
friend_call_back(void* args){
	struct socket_message* ms = (struct socket_message*)args;
	socket_server_broad(ss,ms);
}

//任务线程的mainloop
int 
friend_chat_server_loop(struct socket_message* ms){
	thread_pool_add_task(p,friend_call_back,ms);
	return 1;
}
