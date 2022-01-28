#ifndef __KERNEL_H__
#define __KERNEL_H__
void panic(const char * str);
int printf(const char * fmt, ...);
int printk(const char * fmt, ...);

#endif
