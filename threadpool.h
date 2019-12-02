#ifndef __THREADPOOL_H_
#define __THREADPOOL_H_

#include <pthread.h>
typedef struct  thread_pool_t threadpool_t;


/*创建线程池*/
threadpool_t *thread_pool_create(int min_thr_num, int max_thr_num, int queue_max_size);
/*释放线程池*/
int thread_pool_free(threadpool_t *pool);
/*销毁线程池*/
int thread_pool_destroy(threadpool_t *pool);
/*管理线程*/
void *admin_thread(void *threadpool);
/*线程是否存在*/
int is_thread_alive(pthread_t tid);
/*工作线程*/
void *thread_pool_thread(void *threadpool);
/*向线程池的任务队列中添加一个任务*/
int thread_pool_add_task(threadpool_t *pool, void *(*function)(void *arg), void *arg);

#endif
