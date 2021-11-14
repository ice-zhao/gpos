#ifndef __BSS__H__
#define __BSS__H__

#define NULL ((void*)0)

extern char* _bss;
extern char* _ebss;

void init_bss_section(void);

#define bss_start_ptr  ((char*)&_bss)
#define bss_end_ptr    ((char*)&_ebss)

















#endif
