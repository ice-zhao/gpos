#include "fs/fs.h"
#include "string.h"
#include <kernel/schedule.h>
#include <errno.h>
#include <mm/heap.h>
#include <mm/mm.h>
#include <print.h>
#include <unistd.h>
#include <asm/include/system.h>

long last_pid=0;

int find_empty_process(void)
{
	int i;

repeat:
    if ((++last_pid)<0) last_pid=1;
    for(i=0 ; i<NR_TASKS ; i++)
        if (task[i] && task[i]->pid == last_pid) goto repeat;
	for(i=1 ; i<NR_TASKS ; i++)
		if (!task[i])
			return i;
	return -EAGAIN;
}

int copy_mem(int nr,struct task_struct * p)
{
	unsigned long old_data_base,new_data_base,data_limit;
	unsigned long old_code_base,new_code_base,code_limit;
	code_limit=get_limit(0x0f);	/* ldt selector */
	data_limit=get_limit(0x17);
	old_code_base = get_base(current->ldt[1]);
	old_data_base = get_base(current->ldt[2]);
	if (old_data_base != old_code_base)
		iprintk("We don't support separate I&D\n");
	if (data_limit < code_limit)
		iprintk("Bad data_limit\n");
	new_data_base = new_code_base = nr * 0x4000000;     //64M
	/* p->start_code = new_code_base; */
	set_base(p->ldt[1],new_code_base);
	set_base(p->ldt[2],new_data_base);
	if (copy_page_tables(old_data_base,new_data_base,data_limit)) {
		iprintk("free_page_tables: from copy_mem\n");
		/* free_page_tables(new_data_base,data_limit); */
		return -ENOMEM;
	}
	return 0;
}

/*
 *  Ok, this is the main fork-routine. It copies the system process
 * information (task[nr]) and sets up the necessary registers. It
 * also copies the data segment in it's entirety.
 */
int copy_process(int nr,long ebp,long edi,long esi,long gs,long none,
		long ebx,long ecx,long edx,
		long fs,long es,long ds,
		long eip,long cs,long eflags,long esp,long ss)
{
	struct task_struct *p;
	struct file * f = NULL;
	int i;

	p = (struct task_struct *) get_free_page();
	if (!p)
		return -EAGAIN;
	task[nr] = p;

	*p = *current;	/* NOTE! this doesn't copy the supervisor stack */
	p->state = TASK_UNINTERRUPTIBLE;
	p->pid = last_pid;
    p->father = current->pid;
	p->leader = 0;		/* process leadership doesn't inherit */
	p->counter = p->priority;
	p->tss.back_link = 0;
	p->tss.esp0 = PAGE_SIZE + (long)p;
	p->tss.ss0 = 0x10;
    p->tss.cr3 = (long)pgdir_table_ptr;
	p->tss.eip = eip;
	p->tss.eflags = eflags;
	p->tss.eax = 0;
	p->tss.ecx = ecx;
	p->tss.edx = edx;
	p->tss.ebx = ebx;
	p->tss.esp = esp;
	p->tss.ebp = ebp;
	p->tss.esi = esi;
	p->tss.edi = edi;
	p->tss.es = es & 0xffff;
	p->tss.cs = cs & 0xffff;
	p->tss.ss = ss & 0xffff;
	p->tss.ds = ds & 0xffff;
	p->tss.fs = fs & 0xffff;
	p->tss.gs = gs & 0xffff;
	p->tss.ldt = _LDT(nr);
	p->tss.trace_bitmap = 0x80000000;

	/* if (last_task_used_math == current) */
	/* 	__asm__("clts ; fnsave %0"::"m" (p->tss.i387)); */
	if (copy_mem(nr,p)) {
		task[nr] = NULL;
		free_page((long) p);
		return -EAGAIN;
	}

	for (i=0; i<NR_OPEN;i++)
		if ((f=p->filp[i]))
			f->f_count++;
	if (current->pwd)
		current->pwd->i_count++;
	if (current->root)
		current->root->i_count++;
	if (current->executable)
		current->executable->i_count++;
	set_tss_desc((struct desc_struct *)&gdt+(nr<<1)+FIRST_TSS_ENTRY,&(p->tss));
	set_ldt_desc((struct desc_struct *)&gdt+(nr<<1)+FIRST_LDT_ENTRY,&(p->ldt));
	p->state = TASK_RUNNING;	/* do this last, just in case */
	return last_pid;
}
