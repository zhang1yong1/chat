# chat
 epoll实现socket库实现聊天功能

 1:简单用epoll封装了socket server
 2:封装了线程池来实现任务调度
 3:封装了内存池来实现大小块内存分配[每个.c 文件 MY_MALLC 等换成自己的内存池,jemalloc ,或者自己手写一个]


 简介:
 第一次开源项目,收到建议,给新手写一个网络聊天程序
 服务端：firefly
 客户端：client
 在linux上可以多开，因为是c99标准书写，所以很容易安装在linux 上

 socket_server 库完全是epoll进行封装，2天完成所有模块，所有封装比较简单，可以先看我的源码，
 再查看云风skynet socket_server库，我只用了epoll管理,云风 select 管理服务端fd，epoll管理客户端fd

 threadpool  线程池也是c99标准可动态配置的库，聊天紧紧局内广播作用，简单实用，也是，所有任务调度框架的基础版本，
 新手可以先看，然后，可以github 上tinyactor框架，2级消息队列的actor模型c++11版本，底层也是模仿skynet

 第一版本完成时间：2019/12/2  
