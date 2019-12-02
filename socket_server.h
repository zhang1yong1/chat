#ifndef __SOCKET_SERVER_H__
#define __SOCKET_SERVER_H__

#include <stdint.h>

struct socket_message{
	int   c_fd;
	char* buff;
	int   buff_size;
};
struct socket_server;

#define SOCK_EXIT    -3
#define SOCK_RECVERR -2
#define SOCK_ERROR  -1
#define SOCK_ACCEPT  1
#define SOCK_DATA    2
#define SOCK_CLOSE   3
#define SOCK_SEND    4

struct socket_server* socket_server_create();

int socket_server_bind(struct socket_server* ss,int port);

int socket_server_listen(struct socket_server* ss);

int socket_server_accept(struct socket_server* ss);

int socket_server_poll(struct socket_server* ss,struct socket_message* m);

int socket_server_exit(struct socket_server* ss);

struct socket_message*  socket_message_create();

int socket_server_send(struct socket_server* ss,struct socket_message* m);

int socket_server_broad(struct socket_server* ss,struct socket_message* m);

#endif