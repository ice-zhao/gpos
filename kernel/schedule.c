#include <schedule.h>
#include <bss.h>
#include <asm/include/system.h>

union task_union {
	struct task_struct task;
	char stack[PAGE_SIZE];
};

static union task_union init_task = {INIT_TASK,};

struct task_struct *task[NR_TASKS] = {&(init_task.task), };

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

	ltr(0);
	lldt(0);

}
