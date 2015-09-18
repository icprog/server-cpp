#include"statistics.h"

#ifdef	__cplusplus
extern "C"{
#endif

/* UUID */
EXEC_RETURN uuid_Create(UUID* puuid){
#if defined(WIN32) || defined(_WIN64)
	return CoCreateGuid(puuid) == S_OK ? EXEC_SUCCESS:EXEC_ERROR;
#else
	uuid_generate(puuid->id);
	return EXEC_SUCCESS;
#endif
}
/* 系统信息 */
size_t processor_Count(void){
#if defined(WIN32) || defined(_WIN64)
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	return si.dwNumberOfProcessors;
#else
	long count = sysconf(_SC_NPROCESSORS_CONF);
	if(count < 0)
		count = 0;
	return count;
#endif
}

/* 用户信息 */
EXEC_RETURN user_Name(char* buffer,size_t nbytes){
#if defined(WIN32) || defined(_WIN64)
	DWORD len = nbytes;
	return GetUserNameA(buffer,&len) ? EXEC_SUCCESS:EXEC_ERROR;
#else
	struct passwd pwd,*res = NULL;
	int r = getpwuid_r(getuid(),&pwd,buffer,nbytes,&res);
	if(!res && r){
		errno = r;
		return EXEC_ERROR;
	}
	return EXEC_SUCCESS;
#endif
}

EXEC_RETURN host_Name(char* buffer,size_t nbytes){
#if defined(WIN32) || defined(_WIN64)
	DWORD len = nbytes;
	return GetComputerNameA(buffer,&len) ? EXEC_SUCCESS:EXEC_ERROR;
#else
	return gethostname(buffer,nbytes) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
#endif
}

/* 内存信息 */
EXEC_RETURN page_Size(unsigned long* page_size,unsigned long* granularity){
#if defined(WIN32) || defined(_WIN64)
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	if(page_size)
		*page_size = si.dwPageSize;
	if(granularity)
		*granularity = si.dwAllocationGranularity;
	return EXEC_SUCCESS;
#else
	long value = sysconf(_SC_PAGESIZE);
	if(value == -1)
		return EXEC_ERROR;
	if(page_size)
		*page_size = value;
	if(granularity)
		*granularity = value;
	return EXEC_SUCCESS;
#endif
}
EXEC_RETURN memory_Size(unsigned long long* total,unsigned long long* avail){
#if defined(WIN32) || defined(_WIN64)
	MEMORYSTATUSEX statex = {0};
	statex.dwLength = sizeof(statex);
	if(GlobalMemoryStatusEx(&statex)){
		*total = statex.ullTotalPhys;
		*avail = statex.ullAvailPhys;
		return EXEC_SUCCESS;
	}
	return EXEC_ERROR;
#elif __linux__
	unsigned long page_size,total_page,free_page;
	if((page_size = sysconf(_SC_PAGESIZE)) == -1)
		return EXEC_ERROR;
	if((total_page = sysconf(_SC_PHYS_PAGES)) == -1)
		return EXEC_ERROR;
	if((free_page = sysconf(_SC_AVPHYS_PAGES)) == -1)
		return EXEC_ERROR;
	*total = (unsigned long long)total_page * (unsigned long long)page_size;
	*avail = (unsigned long long)free_page * (unsigned long long)page_size;
	return EXEC_SUCCESS;
#elif __APPLE__
	int64_t value;
	size_t len = sizeof(value);
	if(sysctlbyname("hw.memsize",&value,&len,NULL,0) == -1)
		return EXEC_ERROR;
	*total = value;
	*avail = 0;// sorry...
	return EXEC_SUCCESS;
#endif
}

/* 块设备 */
EXEC_RETURN partition_Size(const char* dev_path,unsigned long long* total_mb,unsigned long long* free_mb,unsigned long long* availabel_mb,unsigned long long* b_size){
#if defined(WIN32) || defined(_WIN64)
	DWORD spc,bps,nfc,tnc;
	ULARGE_INTEGER t,f,a;
	if(GetDiskFreeSpaceExA(dev_path,&a,&t,&f) && GetDiskFreeSpaceA(dev_path,&spc,&bps,&nfc,&tnc)){
		*total_mb = t.QuadPart >> 20;
		*free_mb = f.QuadPart >> 20;
		*availabel_mb = a.QuadPart >> 20;
		*b_size = spc * bps;
		return EXEC_SUCCESS;
	}
	return EXEC_ERROR;
#else
	struct statvfs disk_info = {0};
	if(!statvfs(dev_path,&disk_info)){
		*total_mb = (disk_info.f_blocks * disk_info.f_bsize) >> 20;
		*free_mb = (disk_info.f_bfree * disk_info.f_bsize) >> 20;
		*availabel_mb = (disk_info.f_bavail * disk_info.f_bsize) >> 20;
		*b_size = disk_info.f_bsize;
		return EXEC_SUCCESS;
	}
	return EXEC_ERROR;
#endif
}

#ifdef	__cplusplus
}
#endif
