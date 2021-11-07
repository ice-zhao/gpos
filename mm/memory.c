#include <print.h>
#include <kernel/head.h>
#include <mm/heap.h>
#include <machine.h>
#include <mm/mm.h>
#include <asm/include/system.h>
#include <string.h>

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
		if (!(to_page_table = (unsigned long *)get_free_page()))
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
	return 0;
}


static long memory_end = 0;
static long buffer_memory_end = 4*1024*1024;    //4M
static long main_memory_start = 0;
static long HIGH_MEMORY = 0;
static long PAGING_MEMORY = 0;
static long PAGING_PAGES = 0;
static unsigned long mem_map [ MM_BITMAPS ] = {0,};

void mm_init(void) {
    memory_end = mach_data.total_mem_size<<10;
    main_memory_start = buffer_memory_end;

    HIGH_MEMORY = memory_end;
    PAGING_MEMORY = memory_end - LOW_MEM;
    PAGING_PAGES = PAGING_MEMORY>>12;

    int i;
    for(i=0; i<MM_BITMAPS; i++) {
        mem_map[i]=0xFFFFFFFF;
    }

    unsigned long addr = 0;
    for(addr=main_memory_start; addr<memory_end; addr+=PAGE_SIZE) {
        clear_mem_map(addr);
    }


}


void set_mem_map(unsigned long addr) {
    struct mm_bitpos bp;
    if(!get_bitpos(addr, &bp)) {
        mem_map[bp.pos] |= (1<<bp.idx);
    }
}

void clear_mem_map(unsigned long addr) {
    struct mm_bitpos bp;
    if(!get_bitpos(addr, &bp)) {
        mem_map[bp.pos] &= ~(1<<bp.idx);
    }
}


/*
 * Get physical address of first (actually last :-) free page, and mark it
 * used. If no free pages left, return 0.
 */
unsigned long get_free_page(void)
{
    unsigned long addr;
    register unsigned long pos asm("eax");

    __asm__ ("std; repe; scasl \n\t"
             "je 1f \n\t"
             "movl %%ecx, %%eax \n\t"
             "1: cld \n\t"
             :"=a" (pos)
             :"0" (0xFFFFFFFF), "c" (MM_BITMAPS), "D" (mem_map+MM_BITMAPS-1)
        );

    int idx=sizeof(long)*8;
    if(pos >=0 && pos <MM_BITMAPS) {
        while(--idx >= 0 && (mem_map[pos] & (1<<idx)));
        /* 1bit stands for 4K, 1 long type element has 32bits */
        addr = (pos*sizeof(long)*8 + idx) * 4096 + LOW_MEM;
        memset((void*)addr,0,PAGE_SIZE);
        set_mem_map(addr);
    }
    else
    {
        addr=0;
        iprintk("get free page bad: zero \n");
    }

    return addr;
}
