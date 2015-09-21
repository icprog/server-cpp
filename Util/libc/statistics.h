#ifndef	STATISTICS_H_
#define	STATISTICS_H_

#include"basicdef.h"

#if defined(WIN32) || defined(_WIN64)
	#ifndef	HOST_NAME_MAX
		#define	HOST_NAME_MAX	MAX_COMPUTERNAME_LENGTH
	#endif
#else
	#include<sys/statvfs.h>
	#include<sys/utsname.h>
	#include<pwd.h>
	#include<uuid/uuid.h>
	typedef struct{
		uuid_t id;
	}UUID;
#endif

#ifdef	__cplusplus
extern "C"{
#endif

/* UUID */
EXEC_RETURN uuid_Create(UUID* puuid);
/* 系统信息 */
size_t processor_Count(void);
/* 用户信息 */
EXEC_RETURN user_Name(char* buffer,size_t nbytes);
EXEC_RETURN host_Name(char* buffer,size_t nbytes);
/* 内存信息 */
EXEC_RETURN page_Size(unsigned long* page_size,unsigned long* granularity);
EXEC_RETURN memory_Size(unsigned long long* total,unsigned long long* avail);
/* 块设备 */
EXEC_RETURN partition_Size(const char* dev_path,unsigned long long* total_mb,unsigned long long* free_mb,unsigned long long* availabel_mb,unsigned long long* b_size);

#ifdef	__cplusplus
}
#endif

#endif

