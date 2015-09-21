#include<stdio.h>

#ifdef  __cplusplus
extern "C"{
#endif

size_t url_Encode(const char* src,size_t nlength,char* dst){
	size_t i,j = 0;
	char ch;
	for(i = 0; i < nlength; ++i){
		ch = src[i];
		if(ch >= 'A' && ch <= 'Z'){
			if(dst)
				dst[j] = ch;
			++j;
		}
		else if(ch >= 'a' && ch <= 'z'){
			if(dst)
				dst[j] = ch;
			++j;
		}
        else if (ch >= '0' && ch <= '9'){
			if(dst)
				dst[j] = ch;
			++j;
		}else if(ch == ' '){
			if(dst)
				dst[j] = '+';
			++j;
		}else{
			if(dst)
				sprintf(dst + j,"%%%02X",(unsigned char)ch);
			j += 3;
		}
	}
	if(dst)
		dst[j] = '\0';
	return j;
}

size_t url_Decode(const char* src,size_t nlength,char* dst){
	char ch,ch1,ch2;
	size_t i,j = 0;
	for(i = 0; i < nlength; ++i){
		ch = src[i];
		switch(ch){
			case '+':
				if(dst)
					dst[j] = ' ';
				++j;
				break;
			case '%':
				if(i + 2 < nlength){
					if(src[i+1] >= '0' && src[i+1] <= '9')
						ch1 = src[i+1] - '0';
					else if(src[i+1] >= 'a' && src[i+1] <= 'f')
						ch1 = src[i+1] - 'a' + 10;
					else if(src[i+1] >= 'A' && src[i+1] <= 'F')
						ch1 = src[i+1] - 'A' + 10;
					else
						ch1 = '0';
                    
					if(src[i+2] >= '0' && src[i+2] <= '9')
						ch2 = src[i+2] - '0';
					else if(src[i+2] >= 'a' && src[i+2] <= 'f')
						ch2 = src[i+2] - 'a' + 10;
					else if(src[i+2] >= 'A' && src[i+2] <= 'F')
						ch2 = src[i+2] - 'A' + 10;
					else
						ch2 = '0';
					if(ch1 != '0' && ch2 != '0'){
						if(dst)
							dst[j] = (ch1 << 4) | ch2;
						++j;
						i += 2;
					}
				}
				break;
			default:
				if(dst)
					dst[j] = ch;
				++j;
		}
	}
	if(dst)
		dst[j] = '\0';
	return j;
}

#ifdef  __cplusplus
}
#endif
