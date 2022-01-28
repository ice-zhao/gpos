#include <print.h>
#include <kernel/schedule.h>

void panic(const char * str) {
	printk("Kernel panic! \n");
	printk(str);
	if (current == task[0])
		printk("In swapper task - not syncing\n\r");
	/* else */
	/* 	sys_sync(); */
	for(;;);
}
