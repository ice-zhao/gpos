#define __LIBRARY__

#include <print.h>
#include <bss.h>
#include <kernel/schedule.h>
#include <asm/include/system.h>
#include <mm/heap.h>
#include <asm/include/io.h>
#include <unistd.h>


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




extern void schedule(void);

extern char* _pg_dir;


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
    init_bss_section();
    init_heap();
    schedule_init();
    sti();

    void* task0_user_stack = get_heap(long, 1024);  //4K
    long stack_top = (long)task0_user_stack + 4096;
    move_to_user_mode((long)stack_top);

    if(!fork()) {       //create second process 1
        process1();
    }

    for(;;) {
        process0();     //here is process 0
    }
}
