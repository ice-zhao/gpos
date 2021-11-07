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

#endif
