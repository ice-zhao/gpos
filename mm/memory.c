#include <print.h>
#include <kernel/head.h>
#include <kernel/mm_types.h>
#include <mm/heap.h>
#include <machine.h>
#include <mm/mm.h>
#include <asm/include/system.h>
#include <string.h>
#include <fs/fs.h>
#include <kernel/kernel.h>

#define invalidate() \
    __asm__("movl %%eax,%%cr3"::"a" (&_pg_dir))


static long memory_end = 0;
static const long buffer_memory_end = 4*1024*1024;    //4M
static long main_memory_start = 0;
static long HIGH_MEMORY = 0;
static long PAGING_MEMORY = 0;
static long PAGING_PAGES = 0;
static unsigned long mem_map [ MM_BITMAPS ] = {0,};
static const unsigned long mm_pages_start = buffer_memory_end;
static unsigned long mm_pages_size = 4*1024*1024;   //4M
struct page *mm_pages =(struct page *)mm_pages_start;


/*
 * Free a page of memory at physical address 'addr'. Used by
 * 'free_page_tables()'
 */
void free_page(unsigned long addr)
{
	if (addr < LOW_MEM) return;
	if (addr >= HIGH_MEMORY)
		panic("trying to free nonexistent page\n");
    clear_mem_map(addr);
}


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
	
	if ((from&0x3fffff) || (to&0x3fffff))
		iprintk("copy_page_tables called with wrong alignment\n");
	from_dir = (unsigned long *)(pgdir_table_ptr + ((from>>22) & 0x3ff));
	to_dir = (unsigned long *)(pgdir_table_ptr + ((to>>22) & 0x3ff));
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
			this_page &= ~2;  //for COW
			*to_page_table = this_page;
			if (this_page >= LOW_MEM) {
				*from_page_table = this_page;
                mms_page(this_page)->_refcount++;
			}
		}
	}
	invalidate();
	return 0;
}


void mm_init(void) {
    memory_end = mach_data.total_mem_size<<10;
    main_memory_start = mm_pages_start + mm_pages_size;

    HIGH_MEMORY = memory_end;
    PAGING_MEMORY = memory_end - LOW_MEM;
    PAGING_PAGES = PAGING_MEMORY>>12;

    int i;
    for(i=0; i<MM_BITMAPS; i++) {
        mem_map[i]=0xFFFFFFFF;
    }

    for(i=0; i<MM_PAGES; i++) {
        (mm_pages+i)->_refcount++;
    }

    unsigned long addr = 0;
    for(addr=main_memory_start; addr<memory_end; addr+=PAGE_SIZE) {
        clear_mem_map(addr);
    }

    buffer_init(buffer_memory_end);
}


void set_mem_map(unsigned long addr) {
    struct mm_bitpos bp;
    addr &= 0xfffff000;
    if(!get_bitpos(addr, &bp)) {
        mem_map[bp.pos] |= (1<<bp.idx);
        mms_page(addr)->_refcount++;
    }
}

void clear_mem_map(unsigned long addr) {
    struct mm_bitpos bp;
    if(!get_bitpos(addr, &bp)) {
        --mms_page(addr)->_refcount;
        if(mms_page(addr)->_refcount == 0) {
            mem_map[bp.pos] &= ~(1<<bp.idx);
        } else if (mms_page(addr)->_refcount < 0)
            panic("Try to free free page.\n");
    }
}


/*
 * Get physical address of first (actually last :-) free page, and mark it
 * used. If no free pages left, return 0.
 */
unsigned long get_free_page(void)
{
    unsigned long addr = 0;
    register unsigned long pos asm("eax");

    __asm__ ("std; repe; scasl \n\t"
             "je 1f \n\t"
             "movl %%ecx, %%eax \n\t"
             "1: cld \n\t"
             :"=a" (pos)
             :"0" (0xFFFFFFFF), "c" (MM_BITMAPS), "D" (mem_map+MM_BITMAPS-1)
        );

    if(pos < 0) {
        iprintk("get_free_page: out of memory! \n");
        addr=0;
        return addr;
    }

    int idx=sizeof(long)*8;
    while(--idx >= 0 && (mem_map[pos] & (1<<idx)));
    /* 1bit stands for 4K, 1 long type element has 32bits */
    addr = (pos*sizeof(long)*8 + idx) * 4096 + LOW_MEM;
    memset((void*)addr,0,PAGE_SIZE);
    set_mem_map(addr);

    return addr;
}

void un_wp_page(unsigned long * page_table_entry)
{
	unsigned long old_page,new_page;

	old_page = 0xfffff000 & *page_table_entry;
	if (old_page >= LOW_MEM && mms_page(old_page)->_refcount ==1) {
		*page_table_entry |= 2;
		invalidate();
        return;
	}
	if (!(new_page=get_free_page())) {
        iprintk("out of memory \n");
		/* oom(); */
    }
	if (old_page >= LOW_MEM) {
		mms_page(old_page)->_refcount--;
    }
	*page_table_entry = new_page | 7;
	invalidate();
	copy_page(old_page,new_page);
}

/*
 * This routine handles present pages, when users try to write
 * to a shared page. It is done by copying the page to a new address
 * and decrementing the shared-page counter for the old page.
 *
 * If it's in code space we exit with a segment error.
 */
void do_wp_page(unsigned long error_code,unsigned long address)
{
	un_wp_page(pte_ptr(address));
}
