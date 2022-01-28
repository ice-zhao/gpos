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

#endif
