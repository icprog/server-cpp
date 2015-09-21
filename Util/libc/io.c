#include"io.h"
#include<string.h>

#ifdef	__cplusplus
extern "C"{
#endif

/* 句柄*/
int fd_handle_is_valid(FD_HANDLE fd){
#if defined(WIN32) || defined(_WIN64)
	return fd != INVALID_HANDLE_VALUE && fd > (HANDLE)0;
#else
	return fd >= 0;
#endif	
}

EXEC_RETURN fd_handle_get_inherit_flag(FD_HANDLE fd,int* old_flag){
#if defined(WIN32) || defined(_WIN64)
	DWORD flag;
	if(!GetHandleInformation(fd,&flag))
		return FALSE;
	*old_flag = flag;
	return TRUE;
#else
	int res = fcntl(fd,F_GETFD);
	if(res < 0)
		return EXEC_ERROR;
	*old_flag = res;
	return EXEC_SUCCESS;
#endif
}

EXEC_RETURN fd_handle_enable_inherit_flag(FD_HANDLE fd,int bool_val){
#if defined(WIN32) || defined(_WIN64)
	return SetHandleInformation(fd,HANDLE_FLAG_INHERIT,bool_val ? HANDLE_FLAG_INHERIT:0) ? EXEC_SUCCESS:EXEC_ERROR;
#else
	return fcntl(fd,F_SETFD,bool_val == 0) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
#endif
}

FD_HANDLE fd_handle_dup(FD_HANDLE fd){
#if defined(WIN32) || defined(_WIN64)
	HANDLE new_fd = INVALID_HANDLE_VALUE;
	DuplicateHandle(GetCurrentProcess(),fd,GetCurrentProcess(),&new_fd,0,FALSE,DUPLICATE_SAME_ACCESS);
	return new_fd;
#else
	return dup(fd);
#endif
}

EXEC_RETURN fd_handle_flush(FD_HANDLE fd){
#if defined(WIN32) || defined(_WIN64)
	return FlushFileBuffers(fd) ? EXEC_SUCCESS:EXEC_ERROR;
#else
	return fsync(fd) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
#endif
}

/* 终端 */
#if !defined(WIN32) && !defined(_WIN64)
static int SetTTY(int ttyfd,struct termios* old,int min,int time){
	if(!isatty(ttyfd))
		return -1;
	tcgetattr(ttyfd,old);
	struct termios newt = *old;
	newt.c_lflag &= ~ICANON;
	newt.c_lflag &= ~ECHO;
	newt.c_lflag &= ~ISIG;
	newt.c_cc[VMIN] = min;
	newt.c_cc[VTIME] = time;
	tcsetattr(ttyfd,TCSANOW,&newt);
	return 0;
}
#endif

int t_Kbhit(void){
#if defined(WIN32) || defined(_WIN64)
	return _kbhit();
#else
	struct termios old;
	SetTTY(STDIN_FILENO,&old,0,0);
	char c = -1;
	ssize_t n = read(STDIN_FILENO,&c,1);
	tcsetattr(STDIN_FILENO,TCSANOW,&old);
	return !n ? 0:c;
#endif
}

int t_Getch(void){
#if defined(WIN32) || defined(_WIN64)
	return _getch();
#else
	struct termios old;
	SetTTY(STDIN_FILENO,&old,1,0);
	char c = -1;
	int n = (int)read(STDIN_FILENO,&c,1);
	tcsetattr(STDIN_FILENO,TCSANOW,&old);
	return c;
#endif
}

/* 文件操作 */
const char* file_ExtName(const char* path){
	const char* ext = NULL;
	int i;
	for(i = strlen(path); i >= 0; --i){
		if(path[i] == '.'){
			ext = path + i + 1;
			break;
		}
	}
	return ext;
}

const char* file_FileName(const char* path){
	const char* fname = path;
	int i;
	for(i = strlen(path); i >= 0; --i){
#if defined(WIN32) || defined(_WIN64)
		if(path[i] == '\\'){
#else
		if(path[i] == '/'){
#endif
			fname = path + i + 1;
			break;
		}
	}
	return fname;
}

EXEC_RETURN file_GetTypeByName(const char* path,FILE_TYPE* type){
#if defined(WIN32) || defined(_WIN64)
	DWORD mode = GetFileAttributesA(path);
	if(mode & FILE_ATTRIBUTE_DIRECTORY)
		*type = FILE_TYPE_DIRECTORY;
	else if(mode & FILE_ATTRIBUTE_ARCHIVE)
		*type = FILE_TYPE_REGULAR;
	else if(mode & FILE_ATTRIBUTE_REPARSE_POINT)
		*type = FILE_TYPE_SYMLINK;
	else
		*type = FILE_TYPE_UNKNOW;
	return EXEC_SUCCESS;
#else
	struct stat f_stat;
	EXEC_RETURN res = (lstat(path,&f_stat) == 0 ? EXEC_SUCCESS:EXEC_ERROR);
	if(res == EXEC_SUCCESS) {
		mode_t mode = f_stat.st_mode;
		if(S_ISDIR(mode))
			*type = FILE_TYPE_DIRECTORY;
		else if(S_ISREG(mode))
			*type = FILE_TYPE_REGULAR;
		else if(S_ISLNK(mode))
			*type = FILE_TYPE_SYMLINK;
		else
			*type = FILE_TYPE_UNKNOW;
	}
	return res;
#endif
}

EXEC_RETURN file_GetType(FD_HANDLE fd,FILE_TYPE* type){
#if defined(WIN32) || defined(_WIN64)
	BY_HANDLE_FILE_INFORMATION info;
	DWORD mode;
	if(!GetFileInformationByHandle(fd,&info))
		return EXEC_ERROR;
	mode = info.dwFileAttributes;
	if(mode & FILE_ATTRIBUTE_DIRECTORY)
		*type = FILE_TYPE_DIRECTORY;
	else if(mode & FILE_ATTRIBUTE_ARCHIVE)
		*type = FILE_TYPE_REGULAR;
	else if(mode & FILE_ATTRIBUTE_REPARSE_POINT)
		*type = FILE_TYPE_SYMLINK;
	else
		*type = FILE_TYPE_UNKNOW;
	return EXEC_SUCCESS;
#else
	struct stat f_stat;
	EXEC_RETURN res = (fstat(fd,&f_stat) == 0 ? EXEC_SUCCESS:EXEC_ERROR);
	if(res == EXEC_SUCCESS) {
		mode_t mode = f_stat.st_mode;
		if(S_ISDIR(mode))
			*type = FILE_TYPE_DIRECTORY;
		else if(S_ISREG(mode))
			*type = FILE_TYPE_REGULAR;
		else if(S_ISLNK(mode))
			*type = FILE_TYPE_SYMLINK;
		else
			*type = FILE_TYPE_UNKNOW;
	}
	return res;
#endif
}

EXEC_RETURN file_CreateSymlink(const char* actualpath,const char* sympath){
#if defined(WIN32) || defined(_WIN64)
	DWORD dwAttr,dwFlag;
	dwAttr = GetFileAttributesA(actualpath);
	if(dwAttr == INVALID_FILE_ATTRIBUTES)
		return EXEC_ERROR;
	dwFlag = (dwAttr & FILE_ATTRIBUTE_DIRECTORY) ? SYMBOLIC_LINK_FLAG_DIRECTORY:0;
	return CreateSymbolicLinkA(sympath,actualpath,dwFlag) ? EXEC_SUCCESS:EXEC_ERROR;
#else
	return symlink(actualpath,sympath) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
#endif
}

EXEC_RETURN file_CreateHardLink(const char* existpath,const char* newpath){
#if defined(WIN32) || defined(_WIN64)
	return CreateHardLinkA(newpath,existpath,NULL) ? EXEC_SUCCESS:EXEC_ERROR;
#else
	return link(existpath,newpath) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
#endif
}

EXEC_RETURN file_DeleteHardLink(const char* existpath){
#if defined(WIN32) || defined(_WIN64)
	return DeleteFileA(existpath) ? EXEC_SUCCESS:EXEC_ERROR;
#else
	return unlink(existpath) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
#endif
}

EXEC_RETURN file_HardLinkCount(FD_HANDLE fd,unsigned int* count){
#if defined(WIN32) || defined(_WIN64)
	BY_HANDLE_FILE_INFORMATION info;
	if(!GetFileInformationByHandle(fd,&info))
		return EXEC_ERROR;
	*count = info.nNumberOfLinks;
	return EXEC_SUCCESS;
#else
	struct stat f_stat;
	EXEC_RETURN res = (fstat(fd,&f_stat) == 0 ? EXEC_SUCCESS:EXEC_ERROR);
	if(res == EXEC_SUCCESS)
		*count = f_stat.st_nlink;
	return res;
#endif
}

EXEC_RETURN file_LockExclusive(FD_HANDLE fd,long long offset,long long nbytes,int block_bool){
#if defined(WIN32) || defined(_WIN64)
	DWORD dwFlags;
	OVERLAPPED overlapvar = {0};
	overlapvar.Offset = offset;
	overlapvar.OffsetHigh = offset >> 32;
	dwFlags = LOCKFILE_EXCLUSIVE_LOCK;
	if(!block_bool)
		dwFlags |= LOCKFILE_FAIL_IMMEDIATELY;
	return LockFileEx(fd,dwFlags,0,nbytes,nbytes >> 32,&overlapvar) ? EXEC_SUCCESS:EXEC_ERROR;
#else
	struct flock fl = {0};
	fl.l_type = F_WRLCK;
	fl.l_start = offset;
	fl.l_whence = SEEK_SET;
	fl.l_len = nbytes;
	return fcntl(fd,block_bool ? F_SETLKW:F_SETLK,&fl) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
#endif
}

EXEC_RETURN file_LockShared(FD_HANDLE fd,long long offset,long long nbytes,int block_bool){
#if defined(WIN32) || defined(_WIN64)
	DWORD dwFlags = 0;
	OVERLAPPED overlapvar = {0};
	overlapvar.Offset = offset;
	overlapvar.OffsetHigh = offset >> 32;
	if(!block_bool)
		dwFlags |= LOCKFILE_FAIL_IMMEDIATELY;
	return LockFileEx(fd,dwFlags,0,nbytes,nbytes >> 32,&overlapvar) ? EXEC_SUCCESS:EXEC_ERROR;
#else
	struct flock fl = {0};
	fl.l_type = F_RDLCK;
	fl.l_start = offset;
	fl.l_whence = SEEK_SET;
	fl.l_len = nbytes;
	return fcntl(fd,block_bool ? F_SETLKW:F_SETLK,&fl) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
#endif
}

EXEC_RETURN file_Unlock(FD_HANDLE fd,long long offset,long long nbytes){
#if defined(WIN32) || defined(_WIN64)
	OVERLAPPED overlapvar = {0};
	overlapvar.Offset = offset;
	overlapvar.OffsetHigh = offset >> 32;
	return UnlockFileEx(fd,0,nbytes,nbytes >> 32,&overlapvar) ? EXEC_SUCCESS:EXEC_ERROR;
#else
	struct flock fl = {0};
	fl.l_type = F_UNLCK;
	fl.l_start = offset;
	fl.l_whence = SEEK_SET;
	fl.l_len = nbytes;
	return fcntl(fd,F_SETLK,&fl) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
#endif
}

#if defined(WIN32) || defined(_WIN64)
static void system_time_to_struct_tm(SYSTEMTIME* lst,struct tm* t){
	int sum;
	t->tm_sec = lst->wSecond;
	t->tm_min = lst->wMinute;
	t->tm_hour = lst->wHour;
	t->tm_mday = lst->wDay;
	t->tm_mon = lst->wMonth;
	t->tm_year = lst->wYear;
	t->tm_wday = lst->wDayOfWeek;
	t->tm_isdst = -1;
	switch(lst->wMonth){
		case 1:
			sum = 0;
			break;
		case 2:
			sum = 31;
			break;
		case 3:
			sum = 59;
			break;
		case 4:
			sum = 90;
			break;
		case 5:
			sum = 120;
			break;
		case 6:
			sum = 151;
			break;
		case 7:
			sum = 181;
			break;
		case 8:
			sum = 212;
			break;
		case 9:
			sum = 243;
			break;
		case 10:
			sum = 273;
			break;
		case 11:
			sum = 304;
			break;
		case 12:
			sum = 334;
			break;
	}
	sum += lst->wDay;
	--sum;
	if((lst->wYear % 400 == 0 || (lst->wYear % 4 == 0 && lst->wYear % 100 != 0)) && lst->wMonth > 2)
		++sum;
	t->tm_yday = sum;
}
#endif

EXEC_RETURN file_Time(FD_HANDLE fd,struct tm* last_visit,struct tm* last_modify){
#if defined(WIN32) || defined(_WIN64)
	FILETIME vt,mt;
	TIME_ZONE_INFORMATION tz = {0};
	if(GetTimeZoneInformation(&tz) != TIME_ZONE_ID_INVALID && GetFileTime(fd,NULL,&vt,&mt)){
		SYSTEMTIME utc,loc;
		if(last_visit){
			FileTimeToSystemTime(&vt,&utc);
			SystemTimeToTzSpecificLocalTime(&tz,&utc,&loc);
			system_time_to_struct_tm(&loc,last_visit);
		}
		if(last_modify){
			FileTimeToSystemTime(&mt,&utc);
			SystemTimeToTzSpecificLocalTime(&tz,&utc,&loc);
			system_time_to_struct_tm(&loc,last_modify);
		}
		return EXEC_SUCCESS;
	}
	return EXEC_ERROR;
#else
	struct stat f_stat;
	if(!fstat(fd,&f_stat)){
		if(last_visit){
			localtime_r(&f_stat.st_atime,last_visit);
			last_visit->tm_year += 1900;
			++(last_visit->tm_mon);
		}
		if(last_modify){
			localtime_r(&f_stat.st_mtime,last_modify);
			last_modify->tm_year += 1900;
			++(last_modify->tm_mon);
		}
		return EXEC_SUCCESS;
	}
	return EXEC_ERROR;
#endif
}

long long file_Size(FD_HANDLE fd){
#if defined(WIN32) || defined(_WIN64)
	LARGE_INTEGER n = {0};
	if(fd != INVALID_HANDLE_VALUE && GetFileSizeEx(fd,&n))
		return n.QuadPart;
	else
		return -1;
#else
	struct stat f_stat;
	return fstat(fd,&f_stat) == 0?(f_stat.st_size):(-1);
#endif
}

EXEC_RETURN file_ChangeLength(FD_HANDLE fd,long long length){
#if defined(WIN32) || defined(_WIN64)
	BOOL res;
	LARGE_INTEGER pos = {0};
	LARGE_INTEGER off = {0};
	if(!SetFilePointerEx(fd,off,&pos,FILE_CURRENT))
		return EXEC_ERROR;
	off.QuadPart = length;
	if(!SetFilePointerEx(fd,off,NULL,FILE_BEGIN))
		return EXEC_ERROR;
	res = SetEndOfFile(fd);
	SetFilePointerEx(fd,pos,NULL,FILE_BEGIN);
	return res ? EXEC_SUCCESS:EXEC_ERROR;
#else
	return ftruncate(fd,length) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
#endif
}

/* 文件IO */
FD_HANDLE file_Open(const char* path,int oflag,int new_bool,int aio_bool){
#if defined(WIN32) || defined(_WIN64)
	SECURITY_ATTRIBUTES sa = {0};
	sa.nLength = sizeof(sa);
	sa.bInheritHandle = TRUE;
	return CreateFileA(path,oflag,
						FILE_SHARE_READ|FILE_SHARE_WRITE,&sa,
						new_bool ? CREATE_ALWAYS:OPEN_EXISTING,
						aio_bool ? FILE_FLAG_OVERLAPPED:0,NULL);
#else
	if(new_bool)
		return open(path,oflag|O_WRONLY|O_CREAT|O_TRUNC,S_IRWXU|S_IRWXG|S_IRWXO);
	else
		return open(path,oflag);
#endif
}

int file_Read(FD_HANDLE fd,void* buf,unsigned int nbytes){
#if defined(WIN32) || defined(_WIN64)
	DWORD _readbytes;
	if(ReadFile(fd,buf,nbytes,&_readbytes,NULL))
		return _readbytes;
	else if(GetLastError() == ERROR_BROKEN_PIPE)
		return 0;
	else
		return -1;
#else
	return read(fd,buf,nbytes);
#endif
}

int file_Write(FD_HANDLE fd,const void* buf,unsigned int nbytes){
#if defined(WIN32) || defined(_WIN64)
	DWORD _writebytes = 0;
	if(WriteFile(fd,buf,nbytes,&_writebytes,NULL))
		return _writebytes;
	else
		return -1;
#else
	return write(fd,buf,nbytes);
#endif
}

EXEC_RETURN file_aio_bind(AIO_CB* cb,FD_HANDLE fd,void* buffer,unsigned int nbytes,long long offset){
#if defined(WIN32) || defined(_WIN64)
	HANDLE hEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
	if(!hEvent)
		return EXEC_ERROR;
	memset(&cb->ol,0,sizeof(cb->ol));
	cb->ol.Offset = offset;
	cb->ol.OffsetHigh = offset >> 32;
	cb->ol.hEvent = hEvent;
	cb->hFile = fd;
	cb->buffer = buffer;
	cb->nbytes = nbytes;
	return EXEC_SUCCESS;
#else
	memset(cb,0,sizeof(&cb));
	cb->aio_buf = buffer;
	cb->aio_fildes = fd;
	cb->aio_nbytes = nbytes;
	cb->aio_offset = offset;
	return EXEC_SUCCESS;
#endif
}

EXEC_RETURN file_aio_read(AIO_CB* cb){
#if defined(WIN32) || defined(_WIN64)
	BOOL res = ReadFile(cb->hFile,cb->buffer,cb->nbytes,NULL,&cb->ol);
	if(!res && GetLastError() == ERROR_IO_PENDING)
		return EXEC_SUCCESS;
	return EXEC_ERROR;
#else
	return aio_read(cb) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
#endif
}

EXEC_RETURN file_aio_write(AIO_CB* cb){
#if defined(WIN32) || defined(_WIN64)
	BOOL res = WriteFile(cb->hFile,cb->buffer,cb->nbytes,NULL,&cb->ol);
	if(!res && GetLastError() == ERROR_IO_PENDING)
		return EXEC_SUCCESS;
	return EXEC_ERROR;
#else
	return aio_write(cb) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
#endif
}

EXEC_RETURN file_aio_cancel(AIO_CB* cb,int all_bool){
#if defined(WIN32) || defined(_WIN64)
	return CancelIoEx(cb->hFile,all_bool ? NULL:(&cb->ol)) ? EXEC_SUCCESS:EXEC_ERROR;
#else
	int res = aio_cancel(cb->aio_fildes,all_bool ? NULL:cb);
	return (res == AIO_CANCELED || res == AIO_NOTCANCELED) ? EXEC_SUCCESS:EXEC_ERROR;
#endif
}

int file_aio_is_finish(AIO_CB* cb){
#if defined(WIN32) || defined(_WIN64)
	return HasOverlappedIoCompleted(&cb->ol);
#else
	return aio_error(cb) != EINPROGRESS;
#endif	
}

EXEC_RETURN file_aio_result(AIO_CB* cb,unsigned int* realbytes){
#if defined(WIN32) || defined(_WIN64)
	return GetOverlappedResult(cb->hFile,&cb->ol,(PDWORD)realbytes,TRUE) ? EXEC_SUCCESS:EXEC_ERROR;
#else
	ssize_t res = aio_return(cb);
	if(res < 0){
		*realbytes = 0;
		return EXEC_ERROR;
	}else{
		*realbytes = res;
		return EXEC_SUCCESS;
	}
#endif
}

long long file_Seek(FD_HANDLE fd,long long offset,int whence){
#if defined(WIN32) || defined(_WIN64)
	LARGE_INTEGER pos = {0};
	LARGE_INTEGER off = {0};
	off.QuadPart = offset;
	if(!SetFilePointerEx(fd,off,&pos,whence))
		pos.QuadPart = -1;
	return pos.QuadPart;
#else
	return lseek(fd,offset,whence);
#endif
}

long long file_Tell(FD_HANDLE fd){
#if defined(WIN32) || defined(_WIN64)
	LARGE_INTEGER pos = {0};
	LARGE_INTEGER off = {0};
	if(!SetFilePointerEx(fd,off,&pos,FILE_CURRENT))
		pos.QuadPart = -1;
	return pos.QuadPart;
#else
	return lseek(fd,0,SEEK_CUR);
#endif
}

EXEC_RETURN file_Close(FD_HANDLE fd){
#if defined(WIN32) || defined(_WIN64)
	return CloseHandle(fd) ? EXEC_SUCCESS:EXEC_ERROR;
#else
	return close(fd) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
#endif
}

FD_HANDLE file_mmap_open(const char* path,long long* fsize){
#if defined(WIN32) || defined(_WIN64)
	LARGE_INTEGER n = {0};
	HANDLE mmap;
	HANDLE fd = CreateFileA(path,GENERIC_READ|GENERIC_WRITE,
							FILE_SHARE_READ,NULL,
							OPEN_EXISTING,
							FILE_ATTRIBUTE_NORMAL,NULL);
	if(fd == INVALID_HANDLE_VALUE)
		return INVALID_HANDLE_VALUE;
	
	if(fsize){
		if(GetFileSizeEx(fd,&n))
			*fsize = n.QuadPart;
		else{
			CloseHandle(fd);
			return INVALID_HANDLE_VALUE;
		}
	}
	mmap = CreateFileMappingA(fd,NULL,PAGE_READWRITE,0,0,NULL);
	CloseHandle(fd);
	return mmap;
#else
	int fd = open(path,O_RDWR);
	if(fd != -1 && fsize){
		*fsize = lseek(fd,0,SEEK_END);
		if(*fsize == -1){
			close(fd);
			fd = -1;
		}
	}
	return fd;
#endif
}

unsigned int file_mmap_granularity(void){
#if defined(WIN32) || defined(_WIN64)
	SYSTEM_INFO sinfo = {0};
	GetSystemInfo(&sinfo);
	return sinfo.dwAllocationGranularity;
#else
	long Granularity = sysconf(_SC_PAGESIZE);
	if(Granularity == -1)
		Granularity = 0;
	return Granularity;
#endif
}

void* file_mmap(FD_HANDLE fd,long long offset,size_t nbytes){
	void* addr = NULL;
#if defined(WIN32) || defined(_WIN64)
	addr = MapViewOfFileEx(fd,FILE_MAP_READ|FILE_MAP_WRITE,offset >> 32,offset,nbytes,NULL);
#else
	addr = mmap(NULL,nbytes,PROT_READ|PROT_WRITE,MAP_SHARED,fd,offset);
	if(addr == MAP_FAILED)
		addr = NULL;
#endif
	return addr;
}

EXEC_RETURN file_mmflush(void* addr,size_t nbytes){
#if defined(WIN32) || defined(_WIN64)
	return FlushViewOfFile(addr,nbytes) ? EXEC_SUCCESS:EXEC_ERROR;
#else
	return msync(addr,nbytes,MS_SYNC) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
#endif	
}

EXEC_RETURN file_munmap(void* addr,size_t nbytes){
#if defined(WIN32) || defined(_WIN64)
	FlushViewOfFile(addr,nbytes);
	return UnmapViewOfFile(addr) ? EXEC_SUCCESS:EXEC_ERROR;
#else
	msync(addr,nbytes,MS_SYNC);
	return munmap(addr,nbytes) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
#endif
}

/* 目录 */
EXEC_RETURN dir_Create(const char* path){
#if defined(WIN32) || defined(_WIN64)
	return CreateDirectoryA(path,NULL) ? EXEC_SUCCESS:EXEC_ERROR;
#else
	return mkdir(path,S_IRUSR|S_IWUSR|S_IXUSR) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
#endif
}

EXEC_RETURN dir_CurrentPath(char* buf,size_t n){
#if defined(WIN32) || defined(_WIN64)
	return GetCurrentDirectoryA(n,buf) ? EXEC_SUCCESS:EXEC_ERROR;
#else
	return getcwd(buf,n) ? EXEC_SUCCESS:EXEC_ERROR;
#endif
}

EXEC_RETURN dir_Sheft(const char* dir){
#if defined(WIN32) || defined(_WIN64)
	return SetCurrentDirectoryA(dir) ? EXEC_SUCCESS:EXEC_ERROR;
#else
	return chdir(dir) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
#endif
}

DIRECTORY dir_Open(const char* path){
#if defined(WIN32) || defined(_WIN64)
	size_t len;
	WIN32_FIND_DATAA fd;
	char szFullPath[MAX_PATH] = {0};
	strcpy(szFullPath,path);
	len = strlen(szFullPath);
	if(szFullPath[len - 1] == '\\')
		strcat(szFullPath,"*");
	else if(szFullPath[len - 1] != '*')
		strcat(szFullPath,"\\*");
	return  FindFirstFileA(szFullPath,&fd);
#else
	return opendir(path);
#endif
}

EXEC_RETURN dir_Close(DIRECTORY dir){
#if defined(WIN32) || defined(_WIN64)
	return FindClose(dir) ? EXEC_SUCCESS:EXEC_ERROR;
#else
	return closedir(dir) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
#endif	
}

EXEC_RETURN dir_Read(DIRECTORY dir,DIR_ITEM* item){
#if defined(WIN32) || defined(_WIN64)
	while(FindNextFileA(dir,item)){
		if(!strcmp(".",item->cFileName) || !strcmp("..",item->cFileName))
			continue;
		return EXEC_SUCCESS;
	}
	return EXEC_ERROR;
#else
	struct dirent* i;
	while((i = readdir(dir))){
		if(!strcmp(".",i->d_name) || !strcmp("..",i->d_name))
			continue;
		*item = i;
		return EXEC_SUCCESS;
	}
	return EXEC_ERROR;
#endif
}

char* dir_FileName(DIR_ITEM* item){
#if defined(WIN32) || defined(_WIN64)
	return item->cFileName;
#else
	return (*item)->d_name;
#endif
}



/* PIPE */
void pipe_Open(FD_HANDLE* r,FD_HANDLE* w){
#if defined(WIN32) || defined(_WIN64)
	SECURITY_ATTRIBUTES sa = {0};
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;
	if(!CreatePipe(r,w,&sa,0))
		*r = *w = INVALID_HANDLE_VALUE;
#else
	int fd[2];
	if(pipe(fd) != 0)
		*r = *w = -1;
	else{
		*r = fd[0];
		*w = fd[1];
	}
#endif
}

#ifdef	__cplusplus
}
#endif
