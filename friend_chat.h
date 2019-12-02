#ifndef __FRIEND_CHAT_H__
#define __FRIEND_CHAT_H__

#include "socket_server.h"


void* friend_call_back(void* args);

int friend_chat_server_loop(struct socket_message* ms);

#endif