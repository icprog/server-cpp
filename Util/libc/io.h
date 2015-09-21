#ifndef	IO_H_
#define	IO_H_

#include"basicdef.h"

#if defined(WIN32) || defined(_WIN64)
	#include<conio.h>
	typedef HANDLE 				FD_HANDLE;
	typedef HANDLE				DIRECTORY;
	typedef	WIN32_FIND_DATAA	DIR_ITEM;
	typedef struct{
		OVERLAPPED ol;
		HANDLE hFile;
		void* buffer;
		unsigned int nbytes;
	}AIO_CB;
#else
	#include<sys/stat.h>
	#include<aio.h>
	#include<sys/mman.h>
	#include<dirent.h>
	#include<termios.h>
	typedef	int					FD_HANDLE;
	typedef	DIR*				DIRECTORY;
	typedef	struct dirent*		DIR_ITEM;
	typedef struct aiocb		AIO_CB;
#endif

typedef enum{
	FILE_TYPE_UNKNOW,
	FILE_TYPE_DIRECTORY,
	FILE_TYPE_REGULAR,
	FILE_TYPE_SYMLINK
}FILE_TYPE;

#ifdef	__cplusplus
extern "C"{
#endif

/* 句柄 */
int fd_handle_is_valid(FD_HANDLE fd);
EXEC_RETURN fd_handle_get_inherit_flag(FD_HANDLE fd,int* old_flag);
EXEC_RETURN fd_handle_enable_inherit_flag(FD_HANDLE fd,int bool_val);
FD_HANDLE fd_handle_dup(FD_HANDLE fd);
EXEC_RETURN fd_handle_flush(FD_HANDLE fd);
/* 终端 */
int t_Kbhit(void);
int t_Getch(void);
/* 文件操作 */
const char* file_ExtName(const char* path);
const char* file_FileName(const char* path);
EXEC_RETURN file_GetTypeByName(const char* path,FILE_TYPE* type);
EXEC_RETURN file_GetType(FD_HANDLE fd,FILE_TYPE* type);
EXEC_RETURN file_CreateSymlink(const char* actualpath,const char* sympath);
EXEC_RETURN file_CreateHardLink(const char* existpath,const char* newpath);
EXEC_RETURN file_DeleteHardLink(const char* existpath);
EXEC_RETURN file_HardLinkCount(FD_HANDLE fd,unsigned int* count);
EXEC_RETURN file_LockExclusive(FD_HANDLE fd,long long offset,long long nbytes,int block_bool);
EXEC_RETURN file_LockShared(FD_HANDLE fd,long long offset,long long nbytes,int block_bool);
EXEC_RETURN file_Unlock(FD_HANDLE fd,long long offset,long long nbytes);
EXEC_RETURN file_Time(FD_HANDLE fd,struct tm* last_visit,struct tm* last_modify);
long long file_Size(FD_HANDLE fd);
EXEC_RETURN file_ChangeLength(FD_HANDLE fd,long long length);
/* 文件IO */
FD_HANDLE file_Open(const char* path,int oflag,int new_bool,int aio_bool);
int file_Read(FD_HANDLE fd,void* buf,unsigned int nbytes);
int file_Write(FD_HANDLE fd,const void* buf,unsigned int nbytes);
EXEC_RETURN file_aio_bind(AIO_CB* cb,FD_HANDLE fd,void* buffer,unsigned int nbytes,long long offset);
EXEC_RETURN file_aio_read(AIO_CB* cb);
EXEC_RETURN file_aio_write(AIO_CB* cb);
EXEC_RETURN file_aio_cancel(AIO_CB* cb,int all_bool);
int file_aio_is_finish(AIO_CB* cb);
EXEC_RETURN file_aio_result(AIO_CB* cb,unsigned int* realbytes);
long long file_Seek(FD_HANDLE fd,long long offset,int whence);
long long file_Tell(FD_HANDLE fd);
EXEC_RETURN file_Close(FD_HANDLE fd);
FD_HANDLE file_mmap_open(const char* path,long long* fsize);
unsigned int file_mmap_granularity(void);
void* file_mmap(FD_HANDLE fd,long long offset,size_t nbytes);
EXEC_RETURN file_mmflush(void* addr,size_t nbytes);
EXEC_RETURN file_munmap(void* addr,size_t nbytes);
/* 目录 */
EXEC_RETURN dir_Create(const char* path);
EXEC_RETURN dir_CurrentPath(char* buf,size_t n);
EXEC_RETURN dir_Sheft(const char* dir);
DIRECTORY dir_Open(const char* path);
EXEC_RETURN dir_Close(DIRECTORY dir);
EXEC_RETURN dir_Read(DIRECTORY dir,DIR_ITEM* item);
char* dir_FileName(DIR_ITEM* item);
/* PIPE */
void pipe_Open(FD_HANDLE* r,FD_HANDLE* w);

#ifdef	__cplusplus
}
#endif

#endif
