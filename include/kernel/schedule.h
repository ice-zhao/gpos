#ifndef __SCHEDULE_H__
#define __SCHEDULE_H__
#include <head.h>
#include <mm.h>

#define HZ 100

#define NR_TASKS 8
#define FIRST_TASK task[0]
#define LAST_TASK task[NR_TASKS-1]

extern struct desc_struct *gdt;
extern struct task_struct *task[NR_TASKS];
extern struct task_struct *last_task_used_math;
extern struct task_struct *current;
extern long volatile jiffies;
extern long startup_time;


void schedule_init(void);

struct tss_struct {
	long	back_link;	/* 16 high bits zero */
	long	esp0;
	long	ss0;		/* 16 high bits zero */
	long	esp1;
	long	ss1;		/* 16 high bits zero */
	long	esp2;
	long	ss2;		/* 16 high bits zero */
	long	cr3;
	long	eip;
	long	eflags;
	long	eax,ecx,edx,ebx;
	long	esp;
	long	ebp;
	long	esi;
	long	edi;
	long	es;		/* 16 high bits zero */
	long	cs;		/* 16 high bits zero */
	long	ss;		/* 16 high bits zero */
	long	ds;		/* 16 high bits zero */
	long	fs;		/* 16 high bits zero */
	long	gs;		/* 16 high bits zero */
	long	ldt;		/* 16 high bits zero */
	long	trace_bitmap;	/* bits: trace 0, bitmap 16-31 */
};


struct task_struct {
    /* scheduler uses */
    long counter;
	long priority;
/* ldt for this task 0 - zero 1 - cs 2 - ds&ss */
	struct desc_struct ldt[3];
/* tss for this task */
	struct tss_struct tss;
};


/*
 *  INIT_TASK is used to set up the first task table.
 */
#define INIT_TASK {\
    15,15, \
    {\
        {0,0}, \
/* ldt */	{0xFFFF,0xc0fa00},        /*code segment, DPL:3, G:1,D/B:1-32bit S:1-code/data*/ \
            {0xFFFF,0xc0f200}, /*type: 0:data, W:1-writable*/\
    },           \
    /*tss*/ \
    { 0,PAGE_SIZE+(long)&init_task,0x10,0,0,0,0,(long)(&_pg_dir), \
	  0,0,0,0,0,0,0,0, \
	  0,0,0x17,0x17,0x17,0x17,0x17,0x1B, \
	  _LDT(0),0x80000000 \
	} \
}


/*
 * Entry into gdt where to find first TSS. 0-nul, 1-cs, 2-ds, 3-syscall(Video segment now)
 * 4-TSS0, 5-LDT0, 6-TSS1 etc ...
 */
#define FIRST_TSS_ENTRY 4
#define FIRST_LDT_ENTRY (FIRST_TSS_ENTRY+1)
#define _TSS(n) ((((unsigned long) n)<<4)+(FIRST_TSS_ENTRY<<3))
#define _LDT(n) ((((unsigned long) n)<<4)+(FIRST_LDT_ENTRY<<3))
#define ltr(n) __asm__("ltr %%ax"::"a" (_TSS(n)))
#define lldt(n) __asm__("lldt %%ax"::"a" (_LDT(n)))

/*
 *	switch_to(n) should switch tasks to task nr n, first
 * checking that n isn't the current task, in which case it does nothing.
 * This also clears the TS-flag if the task we switched to has used
 * tha math co-processor latest.
 */
#define switch_to(n) {\
struct {long a,b;} __tmp; \
__asm__("cmpl %%ecx,current\n\t" \
	"je 1f\n\t" \
	"movw %%dx,%1\n\t" \
	"xchgl %%ecx,current\n\t" \
	"ljmp *%0\n\t" \
	"1:" \
	::"m" (*&__tmp.a),"m" (*&__tmp.b), \
	"d" (_TSS(n)),"c" ((long) task[n])); \
}

#endif
