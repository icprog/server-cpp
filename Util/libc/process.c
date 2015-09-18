#include"process.h"

#ifdef	__cplusplus
extern "C"{
#endif

/* 进程 */
EXEC_RETURN process_Exec(PROCESS* process,const char* boot_arg){
#if defined(WIN32) || defined(_WIN64)
	PROCESS_INFORMATION pi;
	STARTUPINFOA si = {0};
	si.cb = sizeof(si);
	si.wShowWindow = TRUE;
	si.dwFlags = STARTF_USESHOWWINDOW;
	if(CreateProcessA(NULL,(char*)boot_arg,NULL,NULL,TRUE,0,NULL,NULL,&si,&pi)){
		CloseHandle(pi.hThread);
		process->handle = pi.hProcess;
		process->id = pi.dwProcessId;
		return EXEC_SUCCESS;
	}
	return EXEC_ERROR;
#else
	*process = fork();
	if(*process < 0)
		return EXEC_ERROR;
	else if(*process == 0){
		char* path = (char*)boot_arg;
		char* arg = strstr(path," ");
		*(arg++) = 0;
		if(execl(path,arg,NULL)<0)
			exit(EXIT_FAILURE);
	}
	return EXEC_SUCCESS;
#endif
}

EXEC_RETURN process_Cancel(PROCESS* process){
#if defined(WIN32) || defined(_WIN64)
	return TerminateProcess(process->handle,0) ? EXEC_SUCCESS:EXEC_ERROR;
#else
	return kill(*process,SIGKILL) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
#endif
}

PID process_Id(PROCESS* process){
#if defined(WIN32) || defined(_WIN64)
	return process ? process->id : GetCurrentProcessId();
#else
	return process ? *process : getpid();
#endif
}

EXEC_RETURN process_TryFreeZombie(PROCESS* process,unsigned char* code){
#if defined(WIN32) || defined(_WIN64)
	DWORD _code;
	if(WaitForSingleObject(process->handle,0) == WAIT_OBJECT_0){
		if(GetExitCodeProcess(process->handle,&_code))
			*code = _code;
		process->id = 0;
		return CloseHandle(process->handle) ? EXEC_SUCCESS:EXEC_ERROR;
	}
	return EXEC_ERROR;
#else
	int status = 0;
	if(waitpid(*process,&status,WNOHANG) > 0){
		if(WIFEXITED(status)){
			*code = WEXITSTATUS(status);
			return EXEC_SUCCESS;
		}
	}
	return EXEC_ERROR;
#endif
}

/* 线程 */
#if defined(WIN32) || defined(_WIN64)
static DWORD WINAPI _platform_thread_entry(PVOID arg){
#else
static void* _platform_thread_entry(void* arg){
#endif
	THREAD* thread = (THREAD*)arg;
	thread->entry(thread);
	return NULL;
}
EXEC_RETURN thread_Create(THREAD* thread,void(*entry)(THREAD* const),void* arg){
	thread->entry = entry;
	thread->arg = arg;
	thread->result = NULL;
#if defined(WIN32) || defined(_WIN64)
	thread->handle = (HANDLE)_beginthreadex(NULL,0,_platform_thread_entry,thread,0,(unsigned int*)(&thread->id));
	return thread->handle != NULL ? EXEC_SUCCESS:EXEC_ERROR;
#else
	int res;
	sigset_t sigset;
	sigfillset(&sigset);
	sigdelset(&sigset,SIGKILL);
	sigdelset(&sigset,SIGSTOP);
	sigdelset(&sigset,SIGCONT);
	res = pthread_sigmask(SIG_SETMASK,&sigset,NULL);
	if(res)
		goto err;
	res = pthread_create(&thread->id,NULL,_platform_thread_entry,thread);
	if(res)
		goto err;
	return EXEC_SUCCESS;
err:
	errno = res;
	return EXEC_ERROR;
#endif
}

void* thread_BootArg(THREAD* thread){return thread->arg;}

TID thread_Id(THREAD* thread){
#if defined(WIN32) || defined(_WIN64)
	return thread ? thread->id : GetCurrentThreadId();
#else
	return thread ? thread->id : pthread_self();
#endif
}

EXEC_RETURN thread_Detach(THREAD* thread){
#if defined(WIN32) || defined(_WIN64)
	return CloseHandle(thread->handle) ? EXEC_SUCCESS:EXEC_ERROR;
#else
	int res = pthread_detach(thread->id);
	if(res)
		errno = res;
	return res ? EXEC_ERROR:EXEC_SUCCESS;
#endif
}

EXEC_RETURN thread_Join(THREAD* thread,void** code){
#if defined(WIN32) || defined(_WIN64)
	if(WaitForSingleObject(thread->handle,INFINITE) == WAIT_OBJECT_0){
		thread->id = 0;
		if(code)
			*code = thread->result;
		return CloseHandle(thread->handle) ? EXEC_SUCCESS:EXEC_ERROR;
	}
	return EXEC_ERROR;
#else
	int res = pthread_join(thread->id,code);
	if(!res){
		thread->id = 0;
		if(code)
			*code = thread->result;
	}
	errno = res;
	return !res ? EXEC_SUCCESS:EXEC_ERROR;
#endif
}

EXEC_RETURN thread_Cancel(THREAD* thread,void* code){
	thread->result = code;
#if defined(WIN32) || defined(_WIN64)
	return TerminateThread(thread->handle,0) ? EXEC_SUCCESS:EXEC_ERROR;
#else
	int res = pthread_cancel(thread->id);
	errno = res;
	return !res ? EXEC_SUCCESS:EXEC_ERROR;
#endif
}

EXEC_RETURN thread_Pause(void){
#if defined(WIN32) || defined(_WIN64)
	return SuspendThread(GetCurrentThread()) != (DWORD)(-1) ? EXEC_SUCCESS:EXEC_ERROR;
#else
	pause();
	return EXEC_SUCCESS;
#endif
}

void thread_Exit(THREAD* thread,void* code){
	thread->result = code;
#if defined(WIN32) || defined(_WIN64)
	_endthreadex(0);
#else
	pthread_exit(NULL);
#endif
}

void thread_Sleep(unsigned int msec){
#if defined(WIN32) || defined(_WIN64)
	SleepEx(msec,TRUE);
#else
	struct timeval tval;
	tval.tv_sec = msec / 1000;
	tval.tv_usec = msec % 1000 * 1000;
	select(0,NULL,NULL,NULL,&tval);
#endif
}

EXEC_RETURN thread_AllocLocalKey(TKEY* key){
#if defined(WIN32) || defined(_WIN64)
	*key = TlsAlloc();
	return *key != TLS_OUT_OF_INDEXES ? EXEC_SUCCESS:EXEC_ERROR;
#else
	int res = pthread_key_create(key,NULL);
	if(res){
		errno = res;
		return EXEC_ERROR;
	}
	return EXEC_SUCCESS;
#endif
}

EXEC_RETURN thread_SetLocalSave(TKEY key,void* value){
#if defined(WIN32) || defined(_WIN64)
	return TlsSetValue(key,value) ? EXEC_SUCCESS:EXEC_ERROR;
#else
	int res;
	res = pthread_setspecific(key,value);
	if(res){
		errno = res;
		return EXEC_ERROR;
	}
	return EXEC_SUCCESS;
#endif
}

void* thread_GetLocalValue(TKEY key){
#if defined(WIN32) || defined(_WIN64)
	return TlsGetValue(key);
#else
	return pthread_getspecific(key);
#endif
}

EXEC_RETURN thread_LocalFree(TKEY key){
#if defined(WIN32) || defined(_WIN64)
	return TlsFree(key) ? EXEC_SUCCESS:EXEC_ERROR;
#else
	int res = pthread_key_delete(key);
	if(res){
		errno = res;
		return EXEC_ERROR;
	}
	return EXEC_SUCCESS;
#endif
}

#ifdef	__cplusplus
}
#endif
