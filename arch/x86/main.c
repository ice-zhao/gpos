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

extern void schedule(void);
extern void hd_init(void);
extern void blk_dev_init(void);
/* hard disk info */
struct drive_info { char dummy[32]; } drive_info;
void init(void);

/*
 * This is set up by the boot-loader.
 */
#define EXT_MEM_K (*(unsigned short *)0x90002)
#define DRIVE_INFO (*(struct drive_info *)0x90080)
#define ORIG_ROOT_DEV (*(unsigned short *)0x901FC)

void process1(void) {
    long start1=jiffies;

    for(;;) {
        long end=jiffies;
        if(end - start1 > 600) {
            iprintk("process 1 is running now! \n");
            start1 = end;
        }
    }
}

void process0(void) {
    long start0=jiffies;

    if(!fork()) {       //create second process 1
        init();
        process1();
    }

    for(;;) {
        long end=jiffies;
        if(end - start0 > 300) {
            iprintk("process 0 is running now! \n");
            start0 = end;
        }
    }
}


void main(void)
{
    ROOT_DEV=0x301;     /*major:3, minor:1 hard disk, first partition*/
    init_machine_data();
    init_bss_section();
    drive_info = DRIVE_INFO;
    init_machine_data();
    trap_init();
    init_heap();
    mm_init();
    schedule_init();
    blk_dev_init();
    hd_init();
    tty_init();
    sti();

    unsigned long stack_top = get_free_page() + 4096;
    move_to_user_mode(stack_top);

    process0();     //here is process 0
}


void init(void) {
    setup(&drive_info);
	(void) open("/dev/tty0",O_RDWR,0);
}
