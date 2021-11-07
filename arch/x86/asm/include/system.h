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
    __asm__ volatile ("movw $104,(%%ecx) \n"                            \
                      "movw %%ax,2(%%ecx) \n"                           \
                      "rorl $16,%%eax \n"                                   \
                      "movb %%al,4(%%ecx) \n"                                     \
                      "movb $"type ",5(%%ecx) \n"                       \
                      "movb $0x00,6(%%ecx) \n"                      \
                      "movb %%ah,7(%%ecx) \n"                       \
                      "rorl $16,%%eax \n"                                   \
                  ::"a" (addr), "c" (n) \
        )



/*0x89: P:1,DPL:0,S:0; type:9-tss */
/*0x82: P:1,DPL:0,S:0; type:2-ldt */
#define set_tss_desc(n,addr) _set_tssldt_desc(((char *) (n)),((int)(addr)),"0x89")
#define set_ldt_desc(n,addr) _set_tssldt_desc(((char *) (n)),((int)(addr)),"0x82")


#define _set_gate(gate_addr,type,dpl,addr) \
__asm__ ("movw %%dx,%%ax\n\t" \
	"movw %0,%%dx\n\t" \
	"movl %%eax,%1\n\t" \
	"movl %%edx,%2" \
	: \
	: "i" ((short) (0x8000+(dpl<<13)+(type<<8))), \
	"o" (*((char *) (gate_addr))), \
	"o" (*(4+(char *) (gate_addr))), \
	"d" ((char *) (addr)),"a" (0x00080000))

#define set_intr_gate(n,addr) \
	_set_gate(&idt[n],14,0,addr)

#define set_trap_gate(n,addr) \
	_set_gate(&idt[n],15,0,addr)

#define set_system_gate(n,addr) \
	_set_gate(&idt[n],15,3,addr)

#define sti() __asm__ ("sti"::)
#define cli() __asm__ ("cli"::)

#endif
