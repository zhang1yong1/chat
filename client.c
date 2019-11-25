//客户端
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <assert.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

int sigign() {
	struct sigaction sa;
	sa.sa_handler = SIG_IGN;
	sigaction(SIGPIPE, &sa, 0);
	return 0;
}

int 
main(int argc, char const *argv[]){
	sigign();

	int sfd; //socket描述符
    int size;   //读取和发送文件的长度
    int r;  //函数返回值
    
    int  len;  //要发送的文件名的长度
    char buf[128]; //数据的缓存
    
    struct sockaddr_in dr;  //网络地址
    
    struct epoll_event ev, events[20]; //ev用于注册事件,events用于回传要处理的事件
    int epid = epoll_create(256);

    //1.建立socket
    sfd=socket(AF_INET,SOCK_STREAM,0);
    if(sfd==-1) 
        printf("1:%m\n"),exit(-1);
    printf("socket成功!\n");
    ev.data.fd = sfd;
    ev.events = EPOLLOUT|EPOLLET;//发送事件
    epoll_ctl(epid,EPOLL_CTL_ADD,sfd,&ev);//事件为读

    //2.连接到服务器
    dr.sin_family=AF_INET;
    dr.sin_port=htons(8888);
    inet_aton("192.168.1.18",&dr.sin_addr);
    r=connect(sfd,(struct sockaddr*)&dr,sizeof(dr));
    if(r==-1) 
        printf("2:%m\n"),close(sfd),exit(-1);    
    printf("connect成功!\n");

    //发送到服务器
    // int a = 10;
    // r = send(sfd,&a,sizeof(a),0);
    // if(r == -1){
    // 	 printf("2:%m\n"),close(sfd),exit(-1);
    // }

    //连接成功后,把sfd放入epoll进行管理
    for (;;)
    {
    	int nfds = epoll_wait(epid,events,20,500);
    	for (int i =0; i < nfds; ++i )
    	{
    		// if (events.)
    		// {
    		// 	/* code */
    		// }

    		if(events[i].events & EPOLLOUT){ //数据向外发送  //关闭的时候,也会走这个分支
    			printf("EPOLLOUT\n");
    			int a = 10;
    			r = send(events[i].data.fd,&a,sizeof(a),0); //2次给空的fd发消息,程序就会崩溃


    			if(r == -1){
    				//关闭socket
    				printf("socket close\n");
    	 			printf("2:%m\n"),close(sfd),exit(-1);
                }
                
                r = send(events[i].data.fd,"abcdefghig",10,0); //2次给空的fd发消息,程序就会崩溃

                if(r == -1){
                    //关闭socket
                    printf("socket close\n");
                    printf("3:%m\n"),close(sfd),exit(-1);
                }

    			printf("write success!\n");
    		}
    		if (events[i].events & EPOLLIN){ //数据读

    			int n;
    			int a;
    			n = read(sfd,&a,sizeof(a));
    			if (n < 0)
    			{
    				printf("read error\n");
    			}
    			printf("data:%d\n", a);
    			ev.data.fd = sfd;
    			ev.events = EPOLLOUT |EPOLLET;
    		}
    	}
    }

failed:
    //6.读取到文件尾，发送0数据包
    close(sfd);
    printf("OK!\n");



}