#ifndef __MM_H__
#define __MM_H__
#define PAGE_SIZE  (4096)

#define LOW_MEM 0x100000        //1M
#define MAP_NR(addr) (((addr)-LOW_MEM)>>12)
#define USED 100
#define MM_BITMAPS      512     //64M, one long stands for 128K(32bit*4K), 128K*8(long)=1M, 64M*8=512(long)
#define MM_ERROR       -1

void mm_init(void);

void set_mem_map(unsigned long addr);
void clear_mem_map(unsigned long addr);
unsigned long get_free_page(void);

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

#endif
