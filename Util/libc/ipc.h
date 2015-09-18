#ifndef	IPC_H_
#define	IPC_H_

#include"basicdef.h"

#if defined(WIN32) || defined(_WIN64)
	#include<process.h>
	typedef struct{
		HANDLE mq_event;
		DWORD thread_id;
	}MSGQUEUE;
	typedef	CRITICAL_SECTION		CS_LOCK;
	typedef CONDITION_VARIABLE		CONDITION;
	typedef HANDLE 					MUTEX;
	typedef struct{
		SRWLOCK lock;
		int type;
	}RWLOCK;
	typedef HANDLE 					SEM;
#else
	#include<sys/ipc.h>
	#include<pthread.h>
	#include<semaphore.h>
	#include<sys/sem.h>
	#include<sys/shm.h>
	#include<sys/socket.h>
	#include<sys/time.h>
	#ifdef __APPLE__
		#include<libkern/OSAtomic.h>
	#endif
	#define	IPC_ALL_ACCESS			0666
	typedef	struct{
		int fd[2];
		pthread_t thread_id;
	}MSGQUEUE;
	typedef	pthread_mutex_t			CS_LOCK;
	typedef pthread_cond_t			CONDITION;
	typedef pthread_mutex_t 		MUTEX;
	typedef pthread_rwlock_t		RWLOCK;
	typedef	sem_t* 					SEM;
#endif

typedef struct TIMER_ID{
#if defined(WIN32) || defined(_WIN64)
	HANDLE id;
	volatile BOOL run_bool;
#elif __linux__
	timer_t id;
#elif __APPLE__
	int kq;
#endif
	unsigned int msec;
	void(*timer_routine)(struct TIMER_ID*);
	void* user_data;
}TIMER_ID;

#ifdef	__cplusplus
extern "C"{
#endif

/* 原子操作 */
int _xchg32(ATOM32* addr,int value);
int _xadd32(ATOM32* addr,int value);
/* 信号 */
Sigfunc_t signal_EnableUnifyHandle(void);
int signal_GetSignalCode(void);
Sigfunc_t signal_Handle(int signo,Sigfunc_t func);
/* 消息队列 */
EXEC_RETURN msg_queue_Create(MSGQUEUE* mq);
EXEC_RETURN msg_queue_Send(MSGQUEUE* mq,unsigned int msg_type,size_t arg1,size_t arg2);
EXEC_RETURN msg_queue_SendOver(MSGQUEUE* mq);
EXEC_RETURN msg_queue_Recv(MSGQUEUE* mq,unsigned int* msg_type,size_t* arg1,size_t* arg2);
EXEC_RETURN msg_queue_Close(MSGQUEUE* mq);
/* 临界区 */
EXEC_RETURN cs_lock_Create(CS_LOCK* cs);
EXEC_RETURN cs_lock_Enter(CS_LOCK* cs,int wait_bool);
EXEC_RETURN cs_lock_Leave(CS_LOCK* cs);
EXEC_RETURN cs_lock_Close(CS_LOCK* cs);
/* 条件变量 */
EXEC_RETURN condition_Create(CONDITION* condition);
EXEC_RETURN condition_Wait(CONDITION* condition,CS_LOCK* cs,unsigned long msec);
EXEC_RETURN condition_WakeThread(CONDITION* condition);
EXEC_RETURN condition_WakeAllThread(CONDITION* condition);
EXEC_RETURN condition_Close(CONDITION* condition);
/* 互斥锁 */
EXEC_RETURN mutex_Create(MUTEX* mutex);
EXEC_RETURN mutex_Lock(MUTEX* mutex,int wait_bool);
EXEC_RETURN mutex_Unlock(MUTEX* mutex);
EXEC_RETURN mutex_Close(MUTEX* mutex);
/* 读写锁 */
EXEC_RETURN rwlock_Create(RWLOCK* rwlock);
EXEC_RETURN rwlock_Rlock(RWLOCK* rwlock);
EXEC_RETURN rwlock_Wlock(RWLOCK* rwlock);
EXEC_RETURN rwlock_Unlock(RWLOCK* rwlock);
EXEC_RETURN rwlock_Close(RWLOCK* rwlock);
/* 信号量 */
SEM sem_Create(const char* name,unsigned int val);
EXEC_RETURN sem_Lock(SEM sem,unsigned long msec);
EXEC_RETURN sem_Unlock(SEM sem);
EXEC_RETURN sem_Close(SEM sem);
/* 共享内存 */
void* share_memory_Create(const char* name,size_t nbytes);
EXEC_RETURN share_memory_Close(void* addr);
/* 定时器 */
EXEC_RETURN timer_Create(TIMER_ID* timer_id,unsigned int msec,void(*timer_routine)(TIMER_ID*),void* arg);
EXEC_RETURN timer_Run(TIMER_ID* timer_id,int run_bool);
EXEC_RETURN timer_Close(TIMER_ID* timer_id);

#ifdef	__cplusplus
}
#endif

#endif
