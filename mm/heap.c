#include <mm/heap.h>


static char* heap;


void init_heap(void) {
    heap = (char*)HEAP_PHY_START;
}

char* __get_heap(unsigned long s, unsigned long a, unsigned long n)
{
	char *tmp;

	heap = (char *)(((unsigned long)heap+(a-1)) & ~(a-1));
	tmp = heap;
	heap += s*n;
	return tmp;
}
