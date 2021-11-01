#include <print.h>
#include <kernel/head.h>
#include <mm/heap.h>



#define invalidate() \
    __asm__("movl %%eax,%%cr3"::"a" (&_pg_dir))



/*
 *  Well, here is one of the most complicated functions in mm. It
 * copies a range of linerar addresses by copying only the pages.
 * Let's hope this is bug-free, 'cause this one I don't want to debug :-)
 *
 * Note! We don't copy just any chunks of memory - addresses have to
 * be divisible by 4Mb (one page-directory entry), as this makes the
 * function easier. It's used only by fork anyway.
 *
 * NOTE 2!! When from==0 we are copying kernel space for the first
 * fork(). Then we DONT want to copy a full page-directory entry, as
 * that would lead to some serious memory waste - we just copy the
 * first 160 pages - 640kB. Even that is more than we need, but it
 * doesn't take any more memory - we don't copy-on-write in the low
 * 1 Mb-range, so the pages can be shared with the kernel. Thus the
 * special case for nr=xxxx.
 */
int copy_page_tables(unsigned long from,unsigned long to,long size)
{
	unsigned long * from_page_table;
	unsigned long * to_page_table;
	unsigned long this_page;
	unsigned long * from_dir, * to_dir;
	unsigned long nr;
    unsigned long *base_dir = (unsigned long*)(&_pg_dir);

	if ((from&0x3fffff) || (to&0x3fffff))
		iprintk("copy_page_tables called with wrong alignment\n");
	from_dir = (unsigned long *)(base_dir + ((from>>22) & 0x3ff)); /* _pg_dir = _end_kernel */
	to_dir = (unsigned long *)(base_dir + ((to>>22) & 0x3ff));
	size = ((unsigned) (size+0x3fffff)) >> 22;
	for( ; size-->0 ; from_dir++,to_dir++) {
		if (1 & *to_dir) {
			iprintk("copy_page_tables: already exist\n");
        }

		if (!(1 & *from_dir))   //if from_dir not exists, then continue
			continue;
		from_page_table = (unsigned long *) (0xfffff000 & *from_dir);
		if (!(to_page_table = (unsigned long *) get_heap(long, 1024)))
			return -1;	/* Out of memory, see freeing */
		*to_dir = ((unsigned long) to_page_table) | 7;
		nr = 1024;
		for ( ; nr-- > 0 ; from_page_table++,to_page_table++) {
			this_page = *from_page_table;
			if (!(1 & this_page))
				continue;
			/* this_page &= ~2; */  //later for COW, but not now!!!
			*to_page_table = this_page;
			/* if (this_page > LOW_MEM) { */
			/* 	*from_page_table = this_page; */
			/* 	this_page -= LOW_MEM; */
			/* 	this_page >>= 12; */
			/* 	mem_map[this_page]++; */
			/* } */
		}
	}
	invalidate();
/* loop: */
    /* goto loop; */
	return 0;
}
