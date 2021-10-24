#include <schedule.h>
#include <bss.h>
#include <asm/include/system.h>
#include <asm/include/io.h>
#include <print.h>

#define LATCH (1193182/HZ)


extern int timer_interrupt(void);

union task_union {
	struct task_struct task;
	char stack[PAGE_SIZE];
};

static union task_union init_task = {INIT_TASK,};

struct task_struct *task[NR_TASKS] = {&(init_task.task), };

long volatile jiffies=0;
long startup_time=0;
struct task_struct *current = &(init_task.task);
struct task_struct *last_task_used_math = NULL;



void schedule_init(void) {
    int i;
	struct desc_struct *p;

	set_tss_desc((struct desc_struct *)&gdt+FIRST_TSS_ENTRY,&(init_task.task.tss));
	set_ldt_desc((struct desc_struct *)&gdt+FIRST_LDT_ENTRY,&(init_task.task.ldt));

    p = (struct desc_struct *)&gdt+2+FIRST_TSS_ENTRY;
	for(i=1;i<NR_TASKS;i++) {
		task[i] = NULL;
		p->a=p->b=0;
		p++;
		p->a=p->b=0;
		p++;
	}

/* Clear NT, so that we won't have troubles with that later on */
	__asm__("pushfl ; andl $0xffffbfff,(%esp) ; popfl");
	ltr(0);
	lldt(0);
    /* setup timer reload value, timer uses this value to output
    100 raising edge square waves per second. */
    outb_p(0x36,0x43);		/* binary, mode 3, LSB/MSB, ch 0 */
	outb_p(LATCH & 0xff , 0x40);	/* LSB */
	outb(LATCH >> 8 , 0x40);	/* MSB */
    set_intr_gate(0x20,&timer_interrupt);
	outb(inb_p(0x21)|0xFF,0x21);   //mask all interrupts
    outb(inb_p(0x21)&~0x01,0x21);   //enable timer interrupt
}


void schedule(void)
{
	int i,next,c;
	struct task_struct ** p;

	while (1) {
		c = -1;
		next = 0;
		i = NR_TASKS;
		p = &task[NR_TASKS];
		while (--i) {
			if (!*--p)
				continue;
			if ((*p)->counter > c)
				c = (*p)->counter, next = i;
		}
		if (c) break;
		for(p = &LAST_TASK ; p > &FIRST_TASK ; --p)
			if (*p)
				(*p)->counter = ((*p)->counter >> 1) +
                    (*p)->priority;
	}
	switch_to(next);
}


void do_timer(long cpl)
{
    //test timer interrupt
    /* static char a[2]={'A','\n'}; */
    /* static int  i=0; */
    /* if (++i == 300) { */
    /*     iprintk(a); */
    /*     a[0] = ++a[0]; */
    /*     i=0; */
    /* } */

	if ((--current->counter)>0) return;
	current->counter=0;
	if (!cpl) return;
	/* schedule(); */
}
