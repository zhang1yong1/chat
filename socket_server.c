#include <stdint.h>
#include "socket_server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <fcntl.h>
#include "cache_alloc.h"



#define MY_MALLOC  malloc
#define MY_FREE    free

#define SOCK_LISTEN  10
#define MAX_EVENT    64
#define MAX_BUFFSIZE 512  //最大buff大小

struct socket_client{
	struct sockaddr_in c_addr;
	socklen_t      c_len;
	int            c_fd;
	void*          p_data;  //自己接受的socket数据

	struct socket_client* next;
};


struct socket_server{
	int fd;
	struct sockaddr_in s_addr;
	int listen;
	int ok;
	int ep_id;//epoll id
	struct epoll_event ev;  
	struct epoll_event events[MAX_EVENT];
	

	//增加链表存放客户端连接的socket
	struct socket_client*  head;
	struct socket_client*  tail;
};

static int
setNonBlocking(int fd) {
	int flag = fcntl(fd, F_GETFL, 0);
	if ( -1 == flag ) {
		return -1;
	}

	fcntl(fd, F_SETFL, flag | O_NONBLOCK);

	return 1;
}

static void 
sock_list_add(struct socket_server* ss, struct socket_client* sc){
	if(ss->head == NULL)
	{
		ss->head = sc;
		ss->tail = sc;
		return;
	}

	if(ss->tail == NULL){
		printf("sock list error\n");
	}

	ss->tail->next = sc ;


}

struct socket_client*
sock_list_find(struct socket_server* ss,int cfd){
	if (ss->head == NULL)
	{
		return NULL;
	}
	struct socket_client* node = NULL;
	for ( node = ss->head; node != NULL ; node = node->next)
	{
		if (node->c_fd == cfd)
		{
			return node;
		}
	}

	return NULL;
}

static void
sock_list_del(struct socket_server* ss,int cfd){
	printf("sock_list_del %d\n",cfd);
	if (ss->head == NULL)
	{
		return ;
	}

	struct socket_client* node = NULL;
	//如果是头结点
	if (ss->head->c_fd == cfd )
	{
		node = ss->head;
		if (ss->head == ss->tail)
		{
			ss->head = NULL;
			ss->tail = NULL;
		}
		else{
			ss->head = ss->head->next;
		}
		//移除epoll事件
		if (epoll_ctl(ss->ep_id, EPOLL_CTL_DEL,node->c_fd, NULL) == -1){
			printf("epoll del error\n");
		}

		close(node->c_fd);
		MY_FREE(node->p_data);
		MY_FREE(node);
		return;
	}
	for (node = ss->head; node != NULL ; node = node->next)
	{
		if(node->next->c_fd == cfd){
			node->next = node->next->next;
			//移除epoll事件
			if (epoll_ctl(ss->ep_id, EPOLL_CTL_DEL,node->c_fd, NULL) == -1){
				printf("epoll del error\n");
			}

			close(node->c_fd);
			MY_FREE(node->p_data);
			MY_FREE(node);
			return;
		}
	}
	printf("dell not find\n");
	return;
}

static void 
sock_list_clear(struct socket_server* ss){
	if (ss->head == NULL)
	{
		return ;
	}
	struct socket_client* node = NULL;
	for(  ; ss->head != NULL ; ss->head = ss->head->next)
	{
		node = ss->head;
		MY_FREE(node->p_data);
		MY_FREE(node);		
	}

	ss->head = NULL;
	ss->tail = NULL;
	return;
}

struct socket_server* 
socket_server_create(){
	struct socket_server* ss = (struct socket_server*)MY_MALLOC(sizeof(struct socket_server));
	int fd = socket(AF_INET,SOCK_STREAM,0);
	if(fd == -1){
		return NULL;
	}
	
	ss->fd = fd;
	ss->ok = 0;
	ss->head = NULL;
	ss->tail = NULL;

	//创建一个epoll
	int efd = epoll_create(256);
	ss->ev.data.fd = fd;
	ss->ev.events = EPOLLIN|EPOLLET;
	ss->ep_id = efd;
	//清理内存
	return ss;
}

struct socket_message*  
socket_message_create(){
	struct socket_message* sm = (struct socket_message*)MY_MALLOC(sizeof(struct socket_message));
	sm->c_fd = 0;
	sm->buff = NULL;

	return sm;
}

int 
socket_server_bind(struct socket_server* ss,int port){
	if(ss == NULL || ss->fd == -1){
		return -1;
	}
	ss->s_addr.sin_family = AF_INET;
	ss->s_addr.sin_port = htons(port);
	inet_aton("0.0.0.0",&ss->s_addr.sin_addr);

	return bind(ss->fd,(struct sockaddr*)&ss->s_addr ,sizeof(ss->s_addr));
}


int 
socket_server_listen(struct socket_server* ss){
	int r =  listen(ss->fd,SOCK_LISTEN);
	if (r == -1)
	{
		return r;
	}

	//监听成功就把server fd 加入到epoll管理
	epoll_ctl(ss->ep_id,EPOLL_CTL_ADD,ss->fd,&ss->ev);
	return r;
}

int 
socket_server_accept(struct socket_server* ss){
	printf("socket accept....\n");
	
	int c_len = sizeof(struct sockaddr_in);
	struct socket_client* sc = (struct socket_client*)MY_MALLOC(sizeof(struct socket_client));
	sc->c_len = c_len;
	int c_fd = accept(ss->fd,(struct sockaddr*)&sc->c_addr,&sc->c_len);
	if (c_fd == -1){
		return -1;
	}
	sc->c_fd = c_fd;

	printf("client connect:%d,IP:%s:%u\n",
            c_fd,inet_ntoa(sc->c_addr.sin_addr),
            ntohs(sc->c_addr.sin_port));

	sock_list_add(ss,sc);
	return c_fd;
}

int 
socket_server_send(struct socket_server* ss,struct socket_message* m){
	printf("socket send ...\n");
	// struct epoll_event ev;
	// ev.data.fd = m->c_fd;
	// ev.data.ptr = (void*)m;
	// ev.events = EPOLLOUT|EPOLLET;

	// int r = epoll_ctl(ss->ep_id,EPOLL_CTL_MOD,m->c_fd,&ev);
	// if( r < 0){
	// 	printf("socket send epoll error %d\n",r);
	// 	return SOCK_ERROR;
	// }
	int sockfd = m->c_fd;
    int r ;
    r = send(sockfd,&m->buff_size,sizeof(m->buff_size),0);
    if (r < 0)
    {
    	printf("send error\n");
    	return SOCK_ERROR;
    }
    r = send(sockfd,m->buff, strlen((char*)m->buff), 0);        //发送数据
    if (r < 0)
    {
    	printf("send error\n");
    	return SOCK_ERROR;
    }

    printf("send finish:%d\n",sockfd);
    
	return SOCK_SEND;
}

int 
socket_server_broad(struct socket_server* ss,struct socket_message* m){
	if (ss->head == NULL)
	{
		return SOCK_SEND;
	}
	struct socket_client* node = NULL;
	for ( node = ss->head; node != NULL ; node = node->next)
	{
		int sockfd = node->c_fd;
	    int r ;
	    r = send(sockfd,&m->buff_size,sizeof(m->buff_size),0);
	    if (r < 0)
	    {
	    	printf("send error\n");
	    	return SOCK_ERROR;
	    }
	    r = send(sockfd,m->buff, strlen((char*)m->buff), 0);        //发送数据
	    if (r < 0)
	    {
	    	printf("send error\n");
	    	return SOCK_ERROR;
	    }
	}

	// free(m->buff);
	// free(m);
	return SOCK_SEND;
}

//修改poll
//增加epoll
//3:增加聊天室功能
int 
socket_server_poll(struct socket_server* ss,struct socket_message* m){
	printf("socket poll....\n");

	for(;;)
	{
		memset(ss->events,0,MAX_EVENT*sizeof(struct epoll_event));
		int nfds = epoll_wait(ss->ep_id,ss->events,MAX_EVENT,500);
		for (int i = 0; i < nfds; ++i)
		{
			if (ss->events[i].events & EPOLLERR || ss->events[i].events & EPOLLHUP )
			{
				//客户端异常关闭
				printf("client connect error:%d\n",ss->events[i].data.fd);
				//清理数据结构
				sock_list_del(ss,ss->events[i].data.fd);
                return SOCK_EXIT;  
			}
			else if(ss->events[i].data.fd == ss->fd) //有新的连接
            {
                int connfd = socket_server_accept(ss);    //accept这个连接
                if (connfd == -1)
                {
                	printf("accept error\n");
                	return SOCK_ERROR;
                }

                printf("new client accept %d\n",connfd);
                //这里connfd是阻塞的
                //后面设置成非阻塞
                // if (setNonBlocking(connfd) < 0)
                // {
                // 	printf("setNonBlocking error\n");
                //     return SOCK_ERROR;
                // }

                ss->ev.data.fd = connfd;
                ss->ev.events=EPOLLIN|EPOLLET;
                if(epoll_ctl(ss->ep_id,EPOLL_CTL_ADD,connfd,&ss->ev) < 0){ //将新的fd添加到epoll的监听队列中
                	printf("epoll_ctl error\n");
                	return SOCK_ERROR;
                }

                return SOCK_ACCEPT;
            }
            else if( ss->events[i].events & EPOLLIN ) //接收到数据，读socket
            {
            	//读数据,设定,协议
            	//数据前4个字节,是数据大小[因为大多数网络协议,都是4个字节head]
            	printf("EPOLLIN fd:%d\n", ss->events[i].data.fd);
            	int a ;
                int n = recv(ss->events[i].data.fd,&a,sizeof(a),MSG_WAITALL);  //读完整的4个字节
                printf("recv a %d\n",a);
                //获取完成的4个字节大小后,就接受这个大小的数据包
                if (n == -1)
                {
                 	printf("recv error:%d\n",ss->events[i].data.fd);
                 	//清理数据结构
					sock_list_del(ss,ss->events[i].data.fd);
					printf("sock_list_del\n");
                 	return SOCK_RECVERR;
                }

                //申请a 数据大小内存
                char* buff = MY_MALLOC(a+1);

                int r = recv(ss->events[i].data.fd, buff, a ,MSG_WAITALL);
                if(r == -1){
                	printf("SOCK_RECVERR recv error\n");
                	return SOCK_RECVERR;
                }
                else if (r == 0)
                {
                	//断开连接
                	printf("client connect close %d\n",ss->events[i].data.fd);
                	//清理数据结构
					sock_list_del(ss,ss->events[i].data.fd);
                	return SOCK_CLOSE;
                }
                else if(r == a){//数据接收完成
                	buff[a] = '\0';
                	m->c_fd = ss->events[i].data.fd;
                	m->buff = buff;
                	m->buff_size = a;
                	return SOCK_DATA;
                }
                else if(r < a){
                	printf("recv data continue:%d : %d\n",r,a);
                	continue;
                }
            }
            else if (ss->events[i].events & EPOLLOUT) //有数据发送,写socket 
            {
            	printf("EPOLL EPOLLOUT\n");
            	struct socket_message* m = (struct socket_message*)ss->events[i].data.ptr;    //取数据
          	    int sockfd = m->c_fd;
          	    int r ;
          	    r = send(sockfd,&m->buff_size,sizeof(m->buff_size),0);
          	    if (r < 0)
          	    {
          	    	printf("send error\n");
          	    	return SOCK_ERROR;
          	    }
                r = send(sockfd,m->buff, strlen((char*)m->buff), 0);        //发送数据
                if (r < 0)
          	    {
          	    	printf("send error\n");
          	    	return SOCK_ERROR;
          	    }

          	    printf("send finish:%d\n",sockfd);
          	    struct epoll_event ev;
				ev.data.fd = sockfd;
				ev.events = EPOLLIN|EPOLLET;

				r = epoll_ctl(ss->ep_id,EPOLL_CTL_MOD,sockfd,&ev);
				if( r < 0){
					printf("poll change error %d\n",r);
					return SOCK_ERROR;
				}

          	    return SOCK_SEND;
            }
		}
	}

}

int 
socket_server_exit(struct socket_server* ss){
	close(ss->fd);
	sock_list_clear(ss);
	MY_FREE(ss);
	return 0;
}
