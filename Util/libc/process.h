#ifndef	PROCESS_H_
#define	PROCESS_H_

#include"basicdef.h"

#if defined(WIN32) || defined(_WIN64)
	#include<process.h>
	typedef struct{HANDLE handle;DWORD id;}PROCESS;
	typedef	DWORD					PID;
	typedef	DWORD					TID;
	typedef	DWORD					TKEY;
#else
	#include<pthread.h>
	#include<ucontext.h>
	typedef	pid_t					PROCESS;
	typedef pid_t     				PID;
	typedef	pthread_t				TID;
	typedef	pthread_key_t			TKEY;
#endif

typedef struct THREAD{
	TID id;
#if defined(WIN32) || defined(_WIN64)
	HANDLE handle;
#endif
	void(*entry)(struct THREAD* const);
	void* arg;
	void* result;
}THREAD;

#ifdef	__cplusplus
extern "C"{
#endif

/* 进程 */
EXEC_RETURN process_Exec(PROCESS* process,const char* boot_arg);
EXEC_RETURN process_Cancel(PROCESS* process);
PID process_Id(PROCESS* process);
EXEC_RETURN process_TryFreeZombie(PROCESS* process,unsigned char* code);
/* 线程 */
EXEC_RETURN thread_Create(THREAD* thread,void(*entry)(THREAD* const),void* arg);
void* thread_BootArg(THREAD* thread);
TID thread_Id(THREAD* thread);
EXEC_RETURN thread_Detach(THREAD* thread);
EXEC_RETURN thread_Join(THREAD* thread,void** code);
EXEC_RETURN thread_Cancel(THREAD* thread,void* code);
EXEC_RETURN thread_Pause(void);
void thread_Exit(THREAD* thread,void* code);
void thread_Sleep(unsigned int msec);

EXEC_RETURN thread_AllocLocalKey(TKEY* key);
EXEC_RETURN thread_SetLocalSave(TKEY key,void* value);
void* thread_GetLocalValue(TKEY key);
EXEC_RETURN thread_LocalFree(TKEY key);

#ifdef	__cplusplus
}
#endif

#endif
