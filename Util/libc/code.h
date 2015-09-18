#ifndef	CCODE_H_
#define	CCODE_H_

#include"basicdef.h"

#ifdef	__cplusplus
extern "C"{
#endif

size_t url_Encode(const char* src,size_t nlength,char* dst);
size_t url_Decode(const char* src,size_t nlength,char* dst);

#ifdef	__cplusplus
}
#endif

#endif
