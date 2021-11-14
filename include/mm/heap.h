#ifndef __HEAP_H__
#define __HEAD_H__

#define HEAP_PHY_START  0x100000    //1M

void init_heap(void);
char *__get_heap(unsigned long s, unsigned long a, unsigned long n);


#define get_heap(type, n)                                       \
	((type *)__get_heap(sizeof(type),__alignof__(type),(n)))

#endif
