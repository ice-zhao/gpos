#ifndef __MM_H__
#define __MM_H__
#include "print.h"
#include "string.h"
#include <kernel/head.h>
#include <kernel/mm_types.h>

#define PAGE_SIZE (4096)
#define PAGE_SHIFT	(12)
extern char* _end_kernel;
extern char* _start_kernel;
#define end_of_kernel		((unsigned long*)&_end_kernel)
#define start_of_kernel		((unsigned long *)&_start_kernel)

#define LOW_MEM 0x100000        //1M
#define MAP_NR(addr) (((addr)-LOW_MEM) >> PAGE_SHIFT)
#define USED 100
#define MM_BITMAPS      512     //64M, one long stands for 128K(32bit*4K), 128K*8(long)=1M, 64M*8=512(long)
#define MM_PAGES        16384       //for 64M memory size; 16384*8byte(sizeof(struct page))=128KB occupy pages area
#define MM_ERROR       -1

/* page table entry attribute */
#define PTE_PRESENT		(0x001)
#define PTE_WRITE		(0x002)
#define PTE_USER		(0x004)	/* allow user space to access */
#define PTE_DIRTY		(0x040)
#define PTE_UWP			(PTE_USER | PTE_WRITE | PTE_PRESENT)

/* page dir entry attribute */
#define PDE_PRESENT		(0x001)
#define PDE_WRITE		(0x002)
#define PDE_USER		(0x004)	/* allow user space to access */
#define PDE_UWP			(PDE_USER | PDE_WRITE | PDE_PRESENT)

typedef unsigned long pde_t;
typedef unsigned long pte_t;

#define PAGE_DIRTY(pte)	(((*pte) & PTE_DIRTY) ? 1 : 0)

void mm_init(void);

void set_mem_map(unsigned long addr);
void clear_mem_map(unsigned long addr);
unsigned long get_free_page(void);
void bread_page(unsigned long address,int dev,int b[4]);
extern void free_page(unsigned long addr);

extern struct page *mm_pages;

struct mm_bitpos {
    unsigned long pos;
    unsigned long idx;
};

static inline int get_bitpos(unsigned long addr, struct mm_bitpos* mmbitpos) {
    int nr = MAP_NR(addr);
    int pos = nr/32;
    int idx = nr % 32;
    if(pos >= MM_BITMAPS)
        return MM_ERROR;
    mmbitpos->pos = pos;
    mmbitpos->idx = idx;
    return 0;
}

#define pgdir_idx(vaddr)     ((vaddr>>22) & 0x3ff)
#define page_idx(vaddr)      ((vaddr>>12) & 0x3ff)
#define pde(vaddr) ((pde_t *)(pgdir_table_ptr + pgdir_idx(vaddr)))

#define page_table_from(pg_dir_entry)                                          \
    ((unsigned long *)((*pg_dir_entry) & 0xfffff000))

#define pte(vaddr) ((pte_t *)(page_table_from(pde(vaddr)) + page_idx(vaddr)))

#define PAGE_NR(phyaddr)       ((phyaddr-LOW_MEM)>>12)

static inline unsigned long page_from(pte_t *pg_table_entry) {
	return (unsigned long)((*pg_table_entry) & 0xfffff000);
}

#define copy_page(from,to) \
    __asm__("cld ; rep ; movsl"::"S" (from),"D" (to),"c" (1024))

static inline unsigned long mms_page_idx(struct mm_bitpos *bp) {
    return bp->pos*32+bp->idx;
}

static inline struct page* mms_page(unsigned long addr) {
    if(PAGE_NR(addr) < MM_PAGES)
        return mm_pages+PAGE_NR(addr);
	iprintk("out of page ranges!\n");
    return NULL;
}
#endif
