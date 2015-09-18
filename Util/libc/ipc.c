#include"ipc.h"

#ifdef	__cplusplus
extern "C"{
#endif

/* 原子操作 */
int _xchg32(ATOM32* addr,int value){
#if defined(WIN32) || defined(_WIN64)
	return InterlockedExchange(addr,value);
#else
	return __sync_lock_test_and_set(addr,value);
#endif
}

int _xadd32(ATOM32* addr,int value){
#if defined(WIN32) || defined(_WIN64)
	int old = InterlockedExchangeAdd(addr,value);
	return old + value;
#elif  __APPLE__
	return OSAtomicAdd32(value,addr);
#else
	return __sync_add_and_fetch(addr,value);
#endif
}

/* 信号 */
#if defined(WIN32) || defined(_WIN64)
	static HANDLE pipefd[2];
#else
	static int pipefd[2];
#endif
static void _unify_signal_handle(int signo){
#if defined(WIN32) || defined(_WIN64)
	DWORD _realbytes;
	DWORD e_code = GetLastError();
	WriteFile(pipefd[1],&signo,sizeof(signo),&_realbytes,NULL);
	SetLastError(e_code);
#else
	int e_code = errno;
	write(pipefd[1],&signo,sizeof(signo));
	errno = e_code;
#endif
}

Sigfunc_t signal_EnableUnifyHandle(void){
#if defined(WIN32) || defined(_WIN64)
	SECURITY_ATTRIBUTES sa = {0};
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;
	return CreatePipe(pipefd,pipefd + 1,&sa,0) ? _unify_signal_handle:SIG_ERR;
#else
	return !pipe(pipefd) ? _unify_signal_handle:SIG_ERR;
#endif
}

int signal_GetSignalCode(void){
	int signo;
#if defined(WIN32) || defined(_WIN64)
	DWORD _realbytes;
	if(!ReadFile(pipefd[0],&signo,sizeof(signo),&_realbytes,NULL))
		signo = 0;
#else
	if(read(pipefd[0],&signo,sizeof(signo)) < 0)
		signo = 0;
#endif
	return signo;
}

Sigfunc_t signal_Handle(int signo,Sigfunc_t func){
#if defined(WIN32) || defined(_WIN64)
	return signal(signo,func);
#else
	struct sigaction act,oact;
	act.sa_handler = func;
	sigfillset(&act.sa_mask);
	act.sa_flags = (signo==SIGALRM) ? 0:SA_RESTART;
	if(sigaction(signo,&act,&oact) < 0)
		return SIG_ERR;
	return oact.sa_handler;
#endif
}

/* 消息队列 */
EXEC_RETURN msg_queue_Create(MSGQUEUE* mq){
#if defined(WIN32) || defined(_WIN64)
	MSG msg;
	mq->mq_event = CreateEventA(NULL,TRUE,FALSE,NULL);
	if(mq->mq_event){
		PeekMessage(&msg,NULL,WM_USER,WM_USER,PM_NOREMOVE);
		mq->thread_id = GetCurrentThreadId();
		return SetEvent(mq->mq_event) ? EXEC_SUCCESS:EXEC_ERROR;
	}
	return EXEC_ERROR;
#else
	mq->thread_id = pthread_self();
	return socketpair(AF_UNIX,SOCK_DGRAM,0,mq->fd) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
#endif
}

EXEC_RETURN msg_queue_Send(MSGQUEUE* mq,unsigned int msg_type,size_t arg1,size_t arg2){
#if defined(WIN32) || defined(_WIN64)
	if(WaitForSingleObject(mq->mq_event,INFINITE) == WAIT_OBJECT_0)
		return PostThreadMessage(mq->thread_id,msg_type + WM_USER,arg1,arg2) ? EXEC_SUCCESS:EXEC_ERROR;
	return EXEC_ERROR;
#else
	size_t msg[3] = {msg_type,arg1,arg2};
	return write(mq->fd[1],msg,sizeof(msg)) == sizeof(msg) ? EXEC_SUCCESS:EXEC_ERROR;
#endif
}

EXEC_RETURN msg_queue_SendOver(MSGQUEUE* mq){
#if defined(WIN32) || defined(_WIN64)
	return PostThreadMessage(mq->thread_id,WM_QUIT,0,0) ? EXEC_SUCCESS:EXEC_ERROR;
#else
	return close(mq->fd[1]) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
#endif
}

EXEC_RETURN msg_queue_Recv(MSGQUEUE* mq,unsigned int* msg_type,size_t* arg1,size_t* arg2){
#if defined(WIN32) || defined(_WIN64)
	MSG msg;
	if(GetMessage(&msg,NULL,0,0) > 0){
		if(msg_type)
			*msg_type = msg.message - WM_USER;
		if(arg1)
			*arg1 = msg.wParam;
		if(arg2)
			*arg2 = msg.lParam;
		return EXEC_SUCCESS;
	}
	return EXEC_ERROR;
#else
	size_t msg[3];
	if(mq->thread_id == pthread_self())
		return read(mq->fd[0],msg,sizeof(msg)) == sizeof(msg) ? EXEC_SUCCESS:EXEC_ERROR;
	else
		return EXEC_ERROR;
#endif
}

EXEC_RETURN msg_queue_Close(MSGQUEUE* mq){
#if defined(WIN32) || defined(_WIN64)
	return CloseHandle(mq->mq_event) ? EXEC_SUCCESS:EXEC_ERROR;
#else
	return close(mq->fd[0]) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
#endif
}

/* 临界区 */
EXEC_RETURN cs_lock_Create(CS_LOCK* cs){
#if defined(WIN32) || defined(_WIN64)
	InitializeCriticalSection(cs);
	return EXEC_SUCCESS;
#else
	int res;
	pthread_mutexattr_t attr;
	res = pthread_mutexattr_init(&attr);
	if(res)
		goto err;
	res = pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_RECURSIVE);
	if(res)
		goto err;
	res = pthread_mutex_init(cs,&attr);
	if(res)
		goto err;
	res = pthread_mutexattr_destroy(&attr);
	if(res)
		goto err;
	return EXEC_SUCCESS;
err:
	errno = res;
	return EXEC_ERROR;
#endif
}

EXEC_RETURN cs_lock_Enter(CS_LOCK* cs,int wait_bool){
#if defined(WIN32) || defined(_WIN64)
	if(wait_bool){
		EnterCriticalSection(cs);
		return EXEC_SUCCESS;
	}
	return TryEnterCriticalSection(cs) ? EXEC_SUCCESS:EXEC_ERROR;
#else
	int res;
	if(wait_bool)
		res = pthread_mutex_lock(cs);	
	else
		res = pthread_mutex_trylock(cs);
	errno = res;
	return res ? EXEC_ERROR:EXEC_SUCCESS;
#endif
}

EXEC_RETURN cs_lock_Leave(CS_LOCK* cs){
#if defined(WIN32) || defined(_WIN64)
	LeaveCriticalSection(cs);
	return EXEC_SUCCESS;
#else
	int res = pthread_mutex_unlock(cs);
	errno = res;
	return res ? EXEC_ERROR:EXEC_SUCCESS;
#endif
}

EXEC_RETURN cs_lock_Close(CS_LOCK* cs){
#if defined(WIN32) || defined(_WIN64)
	DeleteCriticalSection(cs);
	return EXEC_SUCCESS;
#else
	int res = pthread_mutex_destroy(cs);
	errno = res;
	return res ? EXEC_ERROR:EXEC_SUCCESS;
#endif
}

/* 条件变量 */
EXEC_RETURN condition_Create(CONDITION* condition){
#if defined(WIN32) || defined(_WIN64)
	InitializeConditionVariable(condition);
	return EXEC_SUCCESS;
#else
	int res = pthread_cond_init(condition,NULL);
	if(res){
		errno = res;
		return EXEC_ERROR;
	}
	return EXEC_SUCCESS;
#endif
}

EXEC_RETURN condition_Wait(CONDITION* condition,CS_LOCK* cs,unsigned long msec){
#if defined(WIN32) || defined(_WIN64)
	return SleepConditionVariableCS(condition,cs,msec) ? EXEC_SUCCESS:EXEC_ERROR;
#else
	int res;
	if(msec == ~0){
		res = pthread_cond_wait(condition,cs);
		if(res)
			goto err;
	}else{
		struct timeval utc = {0};
		struct timespec time_val = {0};
		if(!gettimeofday(&utc,NULL)){
			time_val.tv_sec = utc.tv_sec + msec / 1000;
			msec %= 1000;
			time_val.tv_nsec = utc.tv_usec * 1000 + msec * 1000000;
			res = pthread_cond_timedwait(condition,cs,&time_val);
			if(res)
				goto err;
		}else
			return EXEC_ERROR;
	}
	return EXEC_SUCCESS;
err:
	errno = res;
	return EXEC_ERROR;
#endif
}

EXEC_RETURN condition_WakeThread(CONDITION* condition){
#if defined(WIN32) || defined(_WIN64)
	WakeConditionVariable(condition);
	return EXEC_SUCCESS;
#else
	int res = pthread_cond_signal(condition);
	if(res){
		errno = res;
		return EXEC_ERROR;
	}
	return EXEC_SUCCESS;
#endif
}

EXEC_RETURN condition_WakeAllThread(CONDITION* condition){
#if defined(WIN32) || defined(_WIN64)
	WakeAllConditionVariable(condition);
	return EXEC_SUCCESS;
#else
	int res = pthread_cond_broadcast(condition);
	if(res){
		errno = res;
		return EXEC_ERROR;
	}
	return EXEC_SUCCESS;
#endif
}

EXEC_RETURN condition_Close(CONDITION* condition){
#if defined(WIN32) || defined(_WIN64)
	return EXEC_SUCCESS;
#else
	int res = pthread_cond_destroy(condition);
	if(res){
		errno = res;
		return EXEC_ERROR;
	}
	return EXEC_SUCCESS;
#endif
}

/* 互斥锁 */
EXEC_RETURN mutex_Create(MUTEX* mutex){
#if defined(WIN32) || defined(_WIN64)
	/* windows mutex will auto release after it's owner thread exit. */
	/* then...if another thread call WaitForSingleObject for that mutex,it will return WAIT_ABANDONED */
	HANDLE res = CreateEvent(NULL,FALSE,TRUE,NULL);/* so,I use event. */
	if(res){
		*mutex = res;
		return EXEC_SUCCESS;
	}
	return EXEC_ERROR;
#else
	int res = pthread_mutex_init(mutex,NULL);
	if(res){
		errno = res;
		return EXEC_ERROR;
	}
	return EXEC_SUCCESS;
#endif
}

EXEC_RETURN mutex_Lock(MUTEX* mutex,int wait_bool){
#if defined(WIN32) || defined(_WIN64)
	return WaitForSingleObject(*mutex,wait_bool ? INFINITE:0) == WAIT_OBJECT_0 ? EXEC_SUCCESS:EXEC_ERROR;
#else
	int res;
	if(wait_bool)
		res = pthread_mutex_lock(mutex);	
	else
		res = pthread_mutex_trylock(mutex);
	errno = res;
	return res ? EXEC_ERROR:EXEC_SUCCESS;
#endif
}

EXEC_RETURN mutex_Unlock(MUTEX* mutex){
#if defined(WIN32) || defined(_WIN64)
	return SetEvent(*mutex) ? EXEC_SUCCESS:EXEC_ERROR;
#else
	int res = pthread_mutex_unlock(mutex);
	errno = res;
	return res ? EXEC_ERROR:EXEC_SUCCESS;
#endif
}

EXEC_RETURN mutex_Close(MUTEX* mutex){
#if defined(WIN32) || defined(_WIN64)
	return CloseHandle(*mutex) ? EXEC_SUCCESS:EXEC_ERROR;
#else
	int res = pthread_mutex_destroy(mutex);
	if(res)
		errno = res;
	return res ? EXEC_ERROR:EXEC_SUCCESS;
#endif
}

/* 读写锁 */
EXEC_RETURN rwlock_Create(RWLOCK* rwlock){
#if defined(WIN32) || defined(_WIN64)
	rwlock->type = 0;
	InitializeSRWLock(&rwlock->lock);
	return EXEC_SUCCESS;
#else
	int res;
	res = pthread_rwlock_init(rwlock,NULL);
	errno = res;
	return res ? EXEC_ERROR:EXEC_SUCCESS;
#endif
}

EXEC_RETURN rwlock_Rlock(RWLOCK* rwlock){
#if defined(WIN32) || defined(_WIN64)
	rwlock->type = 1;
	AcquireSRWLockShared(&rwlock->lock);
	return EXEC_SUCCESS;
#else
	int res = pthread_rwlock_rdlock(rwlock);
	errno = res;
	return res ? EXEC_ERROR:EXEC_SUCCESS;
#endif
}

EXEC_RETURN rwlock_Wlock(RWLOCK* rwlock){
#if defined(WIN32) || defined(_WIN64)
	rwlock->type = 2;
	AcquireSRWLockExclusive(&rwlock->lock);
	return EXEC_SUCCESS;
#else
	int res = pthread_rwlock_wrlock(rwlock);
	errno = res;
	return res ? EXEC_ERROR:EXEC_SUCCESS;
#endif
}

EXEC_RETURN rwlock_Unlock(RWLOCK* rwlock){
#if defined(WIN32) || defined(_WIN64)
	int type = rwlock->type;
	rwlock->type = 0;
	if(type == 1)
		ReleaseSRWLockShared(&rwlock->lock);
	else if(type == 2)
		ReleaseSRWLockExclusive(&rwlock->lock);
	return EXEC_SUCCESS;
#else
	int res = pthread_rwlock_unlock(rwlock);
	errno = res;
	return res ? EXEC_ERROR:EXEC_SUCCESS;
#endif
}

EXEC_RETURN rwlock_Close(RWLOCK* rwlock){
#if defined(WIN32) || defined(_WIN64)
	return EXEC_SUCCESS;
#else
	int res = pthread_rwlock_destroy(rwlock);
	errno = res;
	return res ? EXEC_ERROR:EXEC_SUCCESS;
#endif
}

/* 信号量 */
SEM sem_Create(const char* name,unsigned int val){
#if defined(WIN32) || defined(_WIN64)
	return CreateSemaphoreA(NULL,val,INFINITE,name);
#else
	sem_t* sem = sem_open(name,O_CREAT,IPC_ALL_ACCESS,val);
	if(sem == SEM_FAILED)
		sem = NULL;
	return sem;
#endif
}

EXEC_RETURN sem_Lock(SEM sem,unsigned long msec){
#if defined(WIN32) || defined(_WIN64)
	return WaitForSingleObject(sem,msec) == WAIT_OBJECT_0 ? EXEC_SUCCESS:EXEC_ERROR;
#else
    int res;
	if(!msec)
		res = sem_trywait(sem);
	else if(msec == ~0)
		res = sem_wait(sem);
	else{
#ifndef	__APPLE__
		struct timeval utc = {0};
		struct timespec time_val = {0};
		if(!gettimeofday(&utc,NULL)){
			time_val.tv_sec = utc.tv_sec + msec / 1000;
			msec %= 1000;
			time_val.tv_nsec = utc.tv_usec * 1000 + msec * 1000000;
			res = sem_timedwait(sem,&time_val);
		}
#else
		errno = EINVAL;
#endif
		res = -1;
	}
	return res == 0 ? EXEC_SUCCESS:EXEC_ERROR;
#endif
}

EXEC_RETURN sem_Unlock(SEM sem){
#if defined(WIN32) || defined(_WIN64)
	return ReleaseSemaphore(sem,1,NULL) ? EXEC_SUCCESS:EXEC_ERROR;
#else
	return sem_post(sem) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
#endif
}

EXEC_RETURN sem_Close(SEM sem){
#if defined(WIN32) || defined(_WIN64)
	return CloseHandle(sem) ? EXEC_SUCCESS:EXEC_ERROR;
#else
	return sem_close(sem) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
#endif	
}

/* 共享内存 */
void* share_memory_Create(const char* name,size_t nbytes){
#if defined(WIN32) || defined(_WIN64)
	HANDLE mmap = CreateFileMappingA((HANDLE)INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE,0,nbytes,name);
	if(!mmap)
		return NULL;
	return MapViewOfFileEx(mmap,FILE_MAP_ALL_ACCESS,0,0,nbytes,NULL);
#else
	key_t key = ftok(name,0);
	if(key == (key_t)-1)
		return NULL;
	int id = shmget(key,nbytes,IPC_CREAT|IPC_ALL_ACCESS);
	if(id < 0)
		return NULL;
	void* addr = shmat(id,NULL,0);
	if((size_t)addr == ~0)
		return NULL;
	return addr;
#endif
}

EXEC_RETURN share_memory_Close(void* addr){
#if defined(WIN32) || defined(_WIN64)
	return FlushViewOfFile(addr,0) && UnmapViewOfFile(addr) ? EXEC_SUCCESS:EXEC_ERROR;
#else
	return shmdt(addr) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
#endif
}

/* 定时器 */
#if defined(WIN32) || defined(_WIN64)
static VOID CALLBACK timer_inner_proc(PVOID lpParameter,BOOLEAN TimerOrWaitFired){
	if(TimerOrWaitFired){/* for timer,it should alaways TRUE */
		TIMER_ID* timer_id = (TIMER_ID*)lpParameter;
		if(timer_id->run_bool)
			timer_id->timer_routine(timer_id);
	}
}
#elif __linux__
static void timer_inner_proc(union sigval val){
	TIMER_ID* timer_id = (TIMER_ID*)(val.sival_ptr);
	timer_id->timer_routine(timer_id);
}
#elif __APPLE__
static void* timer_inner_proc(void* arg){
	TIMER_ID* timer_id;
	struct kevent e;
	int kq = (size_t)arg;
	pthread_detach(pthread_self());
	while(1){
		if(kevent(kq,NULL,0,&e,1,NULL) < 0)
			break;
		timer_id = (TIMER_ID*)(e.udata);
		timer_id->timer_routine(timer_id);
	}
	return NULL;
}
#endif

EXEC_RETURN timer_Create(TIMER_ID* timer_id,unsigned int msec,void(*timer_routine)(TIMER_ID*),void* arg){
	timer_id->msec = msec;
	timer_id->timer_routine = timer_routine;
	timer_id->user_data = arg;
#if defined(WIN32) || defined(_WIN64)
	timer_id->run_bool = CreateTimerQueueTimer(&timer_id->id,NULL,timer_inner_proc,(void*)timer_id,msec,msec,WT_EXECUTEDEFAULT);
	return timer_id->run_bool ? EXEC_SUCCESS:EXEC_ERROR;
#elif __linux__
	/* create timer */
	struct sigevent sev = {0};
	sev.sigev_notify = SIGEV_THREAD;
	sev.sigev_value.sival_ptr = (void*)timer_id;
	sev.sigev_notify_function = timer_inner_proc;
	if(timer_create(CLOCK_REALTIME,&sev,&timer_id->id) == -1)
		return EXEC_ERROR;
	/* set timer */
	struct itimerspec ts;
	ts.it_interval.tv_sec = msec / 1000;
	ts.it_interval.tv_nsec = msec % 1000 * 1000000;
	ts.it_value.tv_sec = msec / 1000;
	ts.it_value.tv_nsec = msec % 1000 * 1000000;
	return timer_settime(timer_id->id,0,&ts,NULL) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
#elif __APPLE__
	int res;
	struct kevent e;
	pthread_t tid;
	int kq = kqueue();
	if(kq < 0)
		return EXEC_ERROR;
	EV_SET(&e,0,EVFILT_TIMER,EV_ADD|EV_ENABLE,0,msec,timer_id);
	res = kevent(kq,&e,1,NULL,0,NULL);
	if(res < 0)
		return EXEC_ERROR;
	timer_id->kq = kq;
	res = pthread_create(&tid,NULL,timer_inner_proc,(void*)(size_t)(unsigned int)kq);
	if(res){
		errno = res;
		return EXEC_ERROR;
	}
	return EXEC_SUCCESS;
#endif
}

EXEC_RETURN timer_Run(TIMER_ID* timer_id,int run_bool){
	unsigned int msec = timer_id->msec;
#if defined(WIN32) || defined(_WIN64)
	BOOL res;
	if(run_bool)
		res = ChangeTimerQueueTimer(NULL,timer_id->id,msec,msec);
	else
		res = ChangeTimerQueueTimer(NULL,timer_id->id,-1,-1);
	if(res)
		timer_id->run_bool = run_bool;
	return res ? EXEC_SUCCESS:EXEC_ERROR;
#elif __linux__
	struct itimerspec ts;
	if(run_bool){
		ts.it_interval.tv_sec = msec / 1000;
		ts.it_interval.tv_nsec = msec % 1000 * 1000000;
		ts.it_value.tv_sec = msec / 1000;
		ts.it_value.tv_nsec = msec % 1000 * 1000000;
	}else{
		ts.it_interval.tv_sec = 0;
		ts.it_interval.tv_nsec = 0;
		ts.it_value.tv_sec = 0;
		ts.it_value.tv_nsec = 0;
	}
	return timer_settime(timer_id->id,0,&ts,NULL) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
#elif __APPLE__
	struct kevent e;
	if(run_bool)
		EV_SET(&e,0,EVFILT_TIMER,EV_ENABLE,0,msec,timer_id);
	else
		EV_SET(&e,0,EVFILT_TIMER,EV_DISABLE,0,0,NULL);
	return kevent(timer_id->kq,&e,1,NULL,0,NULL) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
#endif
}

EXEC_RETURN timer_Close(TIMER_ID* timer_id){
#if defined(WIN32) || defined(_WIN64)
	BOOL res = DeleteTimerQueueTimer(NULL,timer_id->id,NULL);
	if(res == FALSE && GetLastError() == ERROR_IO_PENDING)
		res = TRUE;
	return res ? EXEC_SUCCESS:EXEC_ERROR;
#elif __linux__
	return timer_delete(timer_id->id) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
#elif __APPLE__
	return close(timer_id->kq) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
#endif
}

#ifdef	__cplusplus
}
#endif
