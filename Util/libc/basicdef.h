#ifndef	BASICDEF_H_
#define	BASICDEF_H_

#if defined(WIN32) || defined(_WIN64)
	#ifndef	_WIN32_WINNT
		#define	_WIN32_WINNT	0x0601
	#endif
	#ifndef WINVER	
		#define WINVER	_WIN32_WINNT
	#endif
	#ifndef	_WIN32_IE
		#define	_WIN32_IE  0x0700
	#endif
	#include<winsock2.h>
	#include<windows.h>
	#ifdef _MSC_VER
		#ifdef	_WIN64
			typedef long long	ssize_t;
		#else
			typedef int 		ssize_t;
		#endif
	#endif
	typedef	LONG volatile		ATOM32;
	#define GetErrorCode()		GetLastError()
	#define	SetErrorCode(e)		SetLastError(e)
#else
	#ifndef _REENTRANT
		#define	_REENTRANT
	#endif
	#ifdef	__APPLE__
		#ifndef	_XOPEN_SOURCE
			#define	_XOPEN_SOURCE
		#endif
	#endif
	#undef _FILE_OFFSET_BITS
	#define	_FILE_OFFSET_BITS	64
	#include<unistd.h>
	#include<sys/types.h>
	#include<sys/param.h>
	#include<sys/sysctl.h>
	#include<fcntl.h>
	#include<sys/wait.h>
	#include<sys/select.h>
	#include<sys/ioctl.h>
	#define GetErrorCode()		(errno)
	#define	SetErrorCode(e)		((errno) = (e))
	typedef int					BOOL;
	typedef	int volatile		ATOM32;
	#ifndef	TRUE
		#define	TRUE	1
	#endif
	#ifndef	FALSE
		#define	FALSE	0
	#endif
#endif

#include<stdio.h>
#include<stdarg.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<time.h>
#include<signal.h>
typedef	size_t ADDRESS;
typedef void (*Sigfunc_t)(int);
typedef enum{
	EXEC_SUCCESS,
	EXEC_ERROR,
}EXEC_RETURN;

#endif
