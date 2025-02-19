#include "kernel/schedule.h"
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


static unsigned long memory_end = 0;
static const long buffer_memory_end = 4*1024*1024;    //4M
static unsigned long main_memory_start = 0;
static long HIGH_MEMORY = 0;
static long PAGING_MEMORY = 0;
static long PAGING_PAGES = 0;
unsigned long mem_map [ MM_BITMAPS ] = {0,};
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

	if ((from&0x3fffff) || (to&0x3fffff))	/* should be 4M boundary */
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
			if (!(1 & this_page))	/* page not present */
				continue;
			this_page &= ~PTE_WRITE;  //for COW
			*to_page_table = this_page;

			if (this_page >= LOW_MEM) {
				*from_page_table = this_page;
				++(mms_page(this_page)->_refcount);
			}
		}
	}
	invalidate();
	return 0;
}

/*
 * This function puts a page in memory at the wanted address.
 * It returns the physical address of the page gotten, 0 if
 * out of memory (either when trying to access page-table or
 * page.)
 */
unsigned long put_page(unsigned long page,unsigned long address)
{
	unsigned long *page_table;
	unsigned long free_page;
	pde_t *pg_dir_entry = NULL;

/* NOTE !!! This uses the fact that _pg_dir=0 */

	if (page < LOW_MEM || page >= HIGH_MEMORY)
		printk("Trying to put page %p at %p\n",page,address);
	if (mms_page(page)->_refcount != 1)
		printk("mem_map disagrees with %p at %p\n",page,address);
	pg_dir_entry = pde(address);
	if ((*pg_dir_entry) & PDE_PRESENT)
		page_table = page_table_from(pg_dir_entry);
	else {
		if (!(free_page=get_free_page()))
			return 0;
		*pg_dir_entry = free_page | PDE_UWP;
		page_table = (unsigned long *) free_page;
	}
	page_table[page_idx(address)] = page | PTE_UWP;
/* no need for invalidate */
	return page;
}

void get_empty_page(unsigned long address)
{
	unsigned long tmp;

	if (!(tmp=get_free_page()) || !put_page(tmp,address)) {
		free_page(tmp);		/* 0 is ok - ignored */
		/* oom(); */
	}
}

/*
 * try_to_share() checks the page at address "address" in the task "p",
 * to see if it exists, and if it is clean. If so, share it with the current
 * task.
 *
 * NOTE! This assumes we have checked that p != current, and that they
 * share the same executable.
 */
static int try_to_share(unsigned long address, struct task_struct * p)
{
	unsigned long from;
	unsigned long to;
	pte_t * from_page;
	pte_t * to_page;
	unsigned long phys_addr;

	unsigned long code_pos;
	pde_t * from_pg_dir_entry = NULL;
	unsigned long * from_pg_table = NULL;

	pde_t * to_pg_dir_entry = NULL;

	code_pos = p->start_code + address;
	from_pg_dir_entry = pde(code_pos);
	/* is there a page-directory at from? */
	if (!(*from_pg_dir_entry & PDE_PRESENT))
		return 0;

	from_page = pte(code_pos);
/* is the page clean and present? */
	if ((*from_page & (PTE_DIRTY | PTE_PRESENT)) != PTE_PRESENT)	/* dirty and present attribute. */
		return 0;
	phys_addr = page_from(from_page);
	if (phys_addr >= HIGH_MEMORY || phys_addr < LOW_MEM)
		return 0;

	code_pos = current->start_code + address;
	to_pg_dir_entry = pde(code_pos);
	if (!(*to_pg_dir_entry & PDE_PRESENT)) {
		if ((to = get_free_page()))
			*to_pg_dir_entry = to | PDE_UWP;
		/* else */
		/* 	oom(); */
	}

	to_page = pte(code_pos);
	if (PTE_PRESENT & *to_page)
		panic("try_to_share: to_page already exists");
/* share them: write-protect */
	*from_page &= ~PTE_WRITE;
	*to_page = *from_page;
	invalidate();
	set_mem_map(phys_addr);
	return 1;
}

/*
 * share_page() tries to find a process that could share a page with
 * the current one. Address is the address of the wanted page relative
 * to the current data space.
 *
 * We first check if it is at all feasible by checking executable->i_count.
 * It should be >1 if there are other tasks sharing this inode.
 */
static int share_page(unsigned long address)
{
	struct task_struct ** p;

	if (!current->executable)
		return 0;
	if (current->executable->i_count < 2)
		return 0;
	for (p = &LAST_TASK ; p > &FIRST_TASK ; --p) {
		if (!*p)
			continue;
		if (current == *p)
			continue;
		if ((*p)->executable != current->executable)
			continue;
		if (try_to_share(address,*p))
			return 1;
	}
	return 0;
}

void do_no_page(unsigned long error_code,unsigned long address)
{
	int nr[4];
	unsigned long image_offset;
	unsigned long page;
	int block,i;

	address &= 0xfffff000;
	image_offset = address - current->start_code;
	if (!current->executable || image_offset >= current->end_data) {
		get_empty_page(address);
		return;
	}
	if (share_page(image_offset))
		return;
	if (!(page = get_free_page()))
		panic("do_no_page: out of memory! \n");
		/* oom(); */
/* remember that 1 block is used for header */
	block = 1 + image_offset/BLOCK_SIZE;
	for (i=0 ; i<4 ; block++,i++)
		nr[i] = bmap(current->executable,block);
	bread_page(page,current->executable->i_dev,nr);
	i = image_offset + 4096 - current->end_data;
	image_offset = page + 4096;
	while (i-- > 0) {
		image_offset--;
		*(char *)image_offset = 0;
	}
	if (put_page(page,address))
		return;
	free_page(page);
	/* oom(); */
}


void mm_init(void) {
    memory_end = mach_data.total_mem_size<<10;	/* machine data uses K as unit, change it to Byte unit */
    main_memory_start = (long)end_of_kernel + (__get_paging_frames() * 4096);

    HIGH_MEMORY = memory_end;
    PAGING_MEMORY = memory_end - LOW_MEM;
    PAGING_PAGES = PAGING_MEMORY>>12;

    int i;
    for(i=0; i<MM_BITMAPS; i++) {
        mem_map[i]=0xFFFFFFFF;
    }

	memset(start_buffer, 0, buffer_memory_end);
	memset(mm_pages, 0, mm_pages_size);
    for(i=0; i<MM_PAGES; i++) {
        ++((mm_pages+i)->_refcount);
    }

    unsigned long addr = 0;
    for(addr=main_memory_start; addr<memory_end; addr+=PAGE_SIZE) {
        clear_mem_map(addr);
    }

    buffer_init(buffer_memory_end);
}


void set_mem_map(unsigned long addr) {
    struct mm_bitpos bp;
	int error = 0;

    addr &= 0xfffff000;
	error = get_bitpos(addr, &bp);

	if(!error) {
        mem_map[bp.pos] |= (1<<bp.idx);
        mms_page(addr)->_refcount++;
    }
}

void clear_mem_map(unsigned long addr) {
    struct mm_bitpos bp;
	int error = 0;
	error = get_bitpos(addr, &bp);

    if(!error) {
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
    unsigned long pos; /* asm("eax") */;

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

/* unlock write protection page
   if the page table entry is used by multiple processes, allocate a new page.
 */
void un_wp_page(unsigned long * page_table_entry)
{
	unsigned long old_page,new_page;

	old_page = page_from(page_table_entry);
	if (old_page >= LOW_MEM && mms_page(old_page)->_refcount ==1) {
		*page_table_entry |= PTE_WRITE;
		invalidate();
        return;
	}

	if (!(new_page=get_free_page())) {
        iprintk("out of memory \n");
		/* oom(); */
    }

	if (old_page >= LOW_MEM) {
		--(mms_page(old_page)->_refcount);
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
	un_wp_page(pte(address));
}
