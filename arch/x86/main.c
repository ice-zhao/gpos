#include "string.h"
#define __LIBRARY__

#include <print.h>
#include <bss.h>
#include <kernel/schedule.h>
#include <asm/include/system.h>
#include <machine.h>
#include <mm/heap.h>
#include <mm/mm.h>
#include <asm/include/io.h>
#include <unistd.h>
#include <machine.h>
#include <kernel/trap.h>
#include <fs/fs.h>
#include <fcntl.h>
#include <kernel/tty.h>
#include <stdarg.h>
#include <unistd.h>
#include <kernel/kernel.h>
/*
 * we need this inline - forking from kernel space will result
 * in NO COPY ON WRITE (!!!), until an execve is executed. This
 * is no problem, but for the stack. This is handled by not letting
 * main() use the stack at all after fork(). Thus, no function
 * calls - which means inline code for fork too, as otherwise we
 * would use the stack upon exit from 'fork()'.
 *
 * Actually only pause and fork are needed inline, so that there
 * won't be any messing with the stack from main(), but we define
 * some others too.
 */
static inline int fork(void) __attribute__((always_inline));
static inline int pause(void) __attribute__((always_inline));
static inline _syscall0(int,fork)
static inline _syscall0(int,pause)
static inline _syscall1(int,setup,void *,BIOS)
static inline _syscall0(int,sync)

extern void hd_init(void);
extern void blk_dev_init(void);
/* hard disk info */
struct drive_info { char dummy[32]; } drive_info;
void init(void);

static char printbuf[1024];
extern int vsprintf();


/*
 * This is set up by the boot-loader.
 */
#define EXT_MEM_K (*(unsigned short *)0x90002)
#define DRIVE_INFO (*(struct drive_info *)0x90080)
#define ORIG_ROOT_DEV (*(unsigned short *)0x901FC)

void process2(void) {
    unsigned long start1=jiffies;
	unsigned long end1=jiffies;

    for(;;) {
        end1=jiffies;
        if(end1 - start1 > 600) {
            printf("%s\n", "process 2 is running now!");
            start1 = end1;
        }
    }
}

void process1(void) {
    unsigned long start1=jiffies;
	unsigned long end1=jiffies;

	if(!fork()) {       //create second process 2
		process2();
    }

    for(;;) {
        end1=jiffies;
        if(end1 - start1 > 600) {
            /* iprintk("process 1 is running now! \n"); */
            printf("%s\n", "process 1 is running now!");
            start1 = end1;
        }
    }
}

void process0(void) {
    unsigned long start0=jiffies;

    if(!fork()) {       //create second process 1
        init();
		process1();
    }

    for(;;) {
        unsigned long end=jiffies;
        if(end - start0 > 300) {
			/* printf("%s\n", "process 0 is running now! from printf."); */
            /* iprintk("process 0 is running now! \n"); */
            start0 = end;
        }
    }
}

void main(void)
{
    init_bss_section();
	ROOT_DEV=0x301;     /*major:3, minor:1 hard disk, first partition*/
    init_machine_data();
    drive_info = DRIVE_INFO;
	mm_init();
    trap_init();
    init_heap();
    schedule_init();
    blk_dev_init();
    hd_init();
    tty_init();
    sti();

    unsigned long stack_top = get_free_page() + 4096;
    move_to_user_mode(stack_top);
    process0();     //here is process 0
}


int printf(const char *fmt, ...)
{
	va_list args;
	int i;

	va_start(args, fmt);
	write(1,printbuf,i=vsprintf(printbuf, fmt, args));
	va_end(args);
	return i;
}

void init(void) {
    setup(&drive_info);
	(void) open("/dev/tty0",O_RDWR,0);
    (void) dup(0);
    (void) dup(0);
    printf("%s\n","user space can use printf now!");
}
