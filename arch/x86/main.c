#include <print.h>
#include <bss.h>
#include <schedule.h>
#include <asm/include/system.h>
#include <mm/heap.h>
#include <asm/include/io.h>

extern char* _pg_dir;



void main(void)
{
    init_bss_section();
    init_heap();
    schedule_init();
    sti();
    /* long int *p=(long int *)0x100000; */
    /* *p=(long)((struct desc_struct *)&gdt+FIRST_LDT_ENTRY); */

    void* task0_user_stack = get_heap(long, 1024);  //4K
    long stack_top = (long)task0_user_stack + 4096;
    move_to_user_mode((long)stack_top);

    for(;;) {
        iprintk("process 0 is running now! \n");
    loop:
        goto loop;
    }
}
