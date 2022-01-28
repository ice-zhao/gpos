#include <kernel/schedule.h>
#include <kernel/sys.h>
#include <bss.h>
#include <asm/include/system.h>
#include <asm/include/io.h>
#include <print.h>
#include <mm/heap.h>
#include <print.h>
#include <kernel/kernel.h>

#define LATCH (1193182/HZ)


extern int timer_interrupt(void);
extern int system_call(void);

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
	set_system_gate(0x80,&system_call);
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
		while (--i >= 0) {
			if (!*--p)
				continue;
			if ((*p)->state == TASK_RUNNING && (*p)->counter > c)   //find max counter of processes
				c = (*p)->counter, next = i;
		}
		if (c > 0) break;
		for(p = &LAST_TASK ; p >= &FIRST_TASK ; --p)
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
	schedule();
}

void sleep_on(struct task_struct **p)
{
	struct task_struct *tmp;

	if (!p)
		return;
	if (current == &(init_task.task)) {
        iprintk("task[0] trying to sleep");
        /* panic("task[0] trying to sleep"); */
        for(;;);
    }
	tmp = *p;
	*p = current;
	current->state = TASK_UNINTERRUPTIBLE;
	schedule();
	*p = tmp;
	if (tmp)
		tmp->state=TASK_RUNNING;
}

void interruptible_sleep_on(struct task_struct **p)
{
	struct task_struct *tmp;

	if (!p)
		return;
	if (current == &(init_task.task))
		panic("task[0] trying to sleep");
	tmp=*p;
	*p=current;
repeat:	current->state = TASK_INTERRUPTIBLE;
	schedule();
	if (*p && *p != current) {
		(*p)->state = TASK_RUNNING;
		goto repeat;
	}
	*p = tmp;
	if (tmp)
		tmp->state = TASK_RUNNING;
}

void wake_up(struct task_struct **p)
{
	if (p && *p) {
		(*p)->state = TASK_RUNNING;
		*p = NULL;
	}
}

int sys_exit(){};
int sys_read(){};
int sys_write(){};
int sys_close(){};
int sys_waitpid(){};
int sys_creat(){};
int sys_link(){};
int sys_unlink(){};
int sys_execve(){};
int sys_chdir(){};
int sys_time(){};
int sys_mknod(){};
int sys_chmod(){};
int sys_chown(){};
int sys_break(){};
int sys_stat(){};
int sys_lseek(){};
int sys_getpid(){};
int sys_mount(){};
int sys_umount(){};
int sys_setuid(){};
int sys_getuid(){};
int sys_stime(){};
int sys_ptrace(){};
int sys_alarm(){};
int sys_fstat(){};
int sys_pause(){};
int sys_utime(){};
int sys_stty(){};
int sys_gtty(){};
int sys_access(){};
int sys_nice(){};
int sys_ftime(){};
int sys_sync(){};
int sys_kill(){};
int sys_rename(){};
int sys_mkdir(){};
int sys_rmdir(){};
int sys_dup(){};
int sys_pipe(){};
int sys_times(){};
int sys_prof(){};
int sys_brk(){};
int sys_setgid(){};
int sys_getgid(){};
int sys_signal(){};
int sys_geteuid(){};
int sys_getegid(){};
int sys_acct(){};
int sys_phys(){};
int sys_lock(){};
int sys_ioctl(){};
int sys_fcntl(){};
int sys_mpx(){};
int sys_setpgid(){};
int sys_ulimit(){};
int sys_uname(){};
int sys_umask(){};
int sys_chroot(){};
int sys_ustat(){};
int sys_dup2(){};
int sys_getppid(){};
int sys_getpgrp(){};
int sys_setsid(){};
int sys_sigaction(){};
int sys_sgetmask(){};
int sys_ssetmask(){};
int sys_setreuid(){};
int sys_setregid(){};
