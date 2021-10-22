#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#define move_to_user_mode(stack_top)           \
    { __asm__ (                                 \
             "pushl $0x17\n\t"                  \
             "pushl %0\n\t"                     \
             "pushfl\n\t"                       \
             "pushl $0x0f\n\t"                  \
             "pushl $1f\n\t"                    \
             "iret\n"                           \
             "1:\tmovl $0x17,%%eax\n\t"         \
             "movw %%ax,%%ds\n\t"               \
             "movw %%ax,%%es\n\t"               \
             "movw %%ax,%%fs\n\t"               \
             "movl $0x1B,%%eax\n\t"             \
             "movw %%ax,%%gs"                   \
               ::"m" (stack_top):"ax");         \
    }

#define _set_tssldt_desc(n,addr,type)                                   \
    __asm__ volatile ("movw $104,%1 \n"                                 \
                  "movw %%ax,%2 \n"                                     \
                  "rorl $16,%%eax \n"                                   \
                  "movb %%al,%3 \n"                                     \
                  "movb $"type ",%4 \n"                                 \
                  "movb $0x00,%5 \n"                                    \
                  "movb %%ah,%6 \n"                                     \
                  "rorl $16,%%eax \n"                                   \
                  ::"a" (addr), "m" (*(n)), "m" (*(n+2)), "m" (*(n+4)), \
                   "m" (*(n+5)), "m" (*(n+6)), "g" (*(n+7)) \
        )


/*0x89: P:1,DPL:0,S:0; type:9-tss */
/*0x82: P:1,DPL:0,S:0; type:2-ldt */
#define set_tss_desc(n,addr) _set_tssldt_desc(((char *) (n)),((int)(addr)),"0x89")
#define set_ldt_desc(n,addr) _set_tssldt_desc(((char *) (n)),((int)(addr)),"0x82")



#endif
