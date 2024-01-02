#ifndef __STRING_H__
#define __STRING_H__

static inline void * memset(void * s,char c,int count)
{
    __asm__("cld\n\t"
            "rep\n\t"
            "stosb"
            ::"a" (c),"D" (s),"c" (count)
        );
    return s;
}

static inline int strlen(const char * s)
{
register int __res ;
__asm__("cld\n\t"
	"repne\n\t"
	"scasb\n\t"
	"notl %0\n\t"
	"decl %0"
	:"=c" (__res):"D" (s),"a" (0),"0" (0xffffffff));
return __res;
}

static inline int strncmp(char* src, char* dest, int len) {
   while (*src != '\0' && *dest != '\0' && *src == *dest && len > 0) {
	   len--;
	   src++;
	   dest++;
   }

   if (len > 0) {
		if (*src == '\0' && *dest != '\0')
			return -1;
		if (*src != '\0' && *dest == '\0')
			return 1;
		if (*src > *dest)
			return 1;
		if (*src < *dest)
			return -1;
   }

   return 0;
}

extern void DispInt(int val);
#endif
