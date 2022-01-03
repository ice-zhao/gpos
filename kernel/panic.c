#include <print.h>
void panic(const char * str) {
	iprintk("Kernel panic! \n");
	iprintk(str);
	/* if (current == task[0]) */
	/* 	printk("In swapper task - not syncing\n\r"); */
	/* else */
	/* 	sys_sync(); */
	for(;;);
}
