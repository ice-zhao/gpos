	.code32
	.section .text
    .global idt, gdt
    .global iprintk, _start_kernel,_end_kernel,idt
	jmp _start
    .include "pm.inc"
    .include "machine.inc"
    .align 8
/* GDT ------------------------------------------------------------------------------------------------------------------------------------------------------------
                                            段基址            段界    , 属性 */
gdt:
LABEL_GDT:              Descriptor             0,                    0, 0						/* 空描述符 */
LABEL_DESC_FLAT_C:		Descriptor             0,             0x0fffff, (DA_CR  | DA_32 | DA_LIMIT_4K)			/* 0 ~ 4G */
LABEL_DESC_FLAT_RW:		Descriptor             0,             0x0fffff, (DA_DRW | DA_32 | DA_LIMIT_4K)			/* 0 ~ 4G */
LABEL_DESC_VIDEO:		Descriptor         0x0B8000,           0x0ffff, (DA_DRW | DA_DPL3)	/* 显存首地址 */
	.fill 252,8,0			/* space for LDT's and TSS's etc */

/* GDT ------------------------------------------------------------------------------------------------------------------------------------------------------------*/
    .equ	GdtLen, (. - LABEL_GDT) /* GDT length */
GdtPtr:
    .2byte (GdtLen - 1) /* GDT limit */
    .4byte (KERNEL_PHY_ADDR + LABEL_GDT)    /* GDT base */

    /* GDT selector */
    .equ 	SelectorFlatC, (LABEL_DESC_FLAT_C	- LABEL_GDT)
    .equ	SelectorFlatRW, (LABEL_DESC_FLAT_RW	- LABEL_GDT)
    .equ	SelectorVideo, (LABEL_DESC_VIDEO	- LABEL_GDT + SA_RPL3)

idt_descr:
   	.2byte 256*8-1                           # idt contains 256 entries, every entry is 8 bytes, limit
	.4byte (idt + KERNEL_PHY_ADDR)           # idt base phy address

idt:	.fill 256,8,0		# idt is uninitialized

_start:
    mov $SelectorFlatRW, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %ss
    movl $0x90000, %esp
    mov $(SelectorVideo), %ax
    mov %ax, %gs /* Video segment selector(dest) */

    lgdtl GdtPtr

	call setup_idt
    call setup_paging

#    int 0x80

#    pushl $int_msg
#    call iprintk
#    add $4, %esp

    movl $((80 * 11 + 0) * 2), %edi
    movb $0xC, %ah /* 0000: Black Back 1100: Red Front */
    movb $'K', %al
    mov %ax, %gs:(%edi)

    pushl $main
    ret
    hlt     #never come here

setup_paging:
    xor %edx, %edx      #calculate the number of pages are used for page table
    movl (memory_size_addr), %eax   #memory size in KB
    movl $1024, %ecx
    divl %ecx
    cmp $0, %edx
    je .calcuate_pages
    inc %eax        #memory size in MB
.calcuate_pages:
    xor %edx, %edx
    movl $4, %ecx       #4 means 4M, every page table can address 4M space.
    divl %ecx       #now eax contains the number of pages needed for page table
    inc %eax    #add one page for page directory

    movl %eax, (paging_frames)  #eax contains the number of page frames. pages*4K/4Btye
    movl $1024, %ecx        #allocate pages at the end of kernel and initialize them to zero
    xor %edx, %edx
    mull %ecx       #edx:eax
    movl %eax, %ecx
    movl $_end_kernel, %edi
    xor %eax, %eax
    cld
    rep
    stosl

    movl $_end_kernel, %edi     #init page directory table
    movl %edi, (paging_addr)    #save paging address to machine data area
    movl (paging_frames), %ecx
    dec %ecx    #minus one page that used for page directory table
    movl $(_end_kernel + PG_P +  PG_USU + PG_RWW), %eax
1:  add $4096, %eax
    movl %eax, (%edi)
    add $4, %edi
    dec %ecx
    jnz 1b

    movl $(_end_kernel + 4096), %edi    #init page tables
    movl (paging_frames), %eax
    dec %eax    #the number of page tables
    xor %edx, %edx
    mov $1024, %ecx
    mull %ecx
    movl %eax, %ecx #ecx contains the number of double word to init
    movl $(PG_P + PG_USU + PG_RWW), %eax
2:  movl %eax, (%edi)
    add $4, %edi
    add $4096, %eax
    dec %ecx
    jnz 2b

    movl (paging_addr), %eax    #enable paging
    movl %eax, %cr3
    movl %cr0, %eax
    orl $0x80000000, %eax
    movl %eax, %cr0
    nop  #give cpu a very little bit delay to take paging effect
    ret


/* This is the default interrupt "handler" :-) */
_int_msg:
    .asciz 	"Unknown interrupt\n"
    .equ int_msg, (KERNEL_PHY_ADDR + _int_msg)
    .align 2
ignore_int:
	pushl %eax
	pushl %ecx
	pushl %edx
	push %ds
	push %es
	push %fs
	movl $SelectorFlatRW,%eax
	mov %ax,%ds
	mov %ax,%es
	mov %ax,%fs
	pushl $int_msg
	call iprintk
	add $4, %esp
	pop %fs
	pop %es
	pop %ds
	popl %edx
	popl %ecx
	popl %eax
	iret


/*
 *  setup_idt
 *
 *  sets up a idt with 256 entries pointing to
 *  ignore_int, interrupt gates. It then loads
 *  idt. Everything that wants to install itself
 *  in the idt-table may do so themselves. Interrupts
 *  are enabled elsewhere, when we can be relatively
 *  sure everything is ok. This routine will be over-
 *  written by the page tables.
0x8E00: means that P=1 Segment Present ; DPL = 0 ring0 ;
    D=1 32bit gate
	  15                           0
	+---+---+---+---+---+---+---+---+
	| P |  DPL  | 0D110 |0000 0000  |
	+---+---+---+---+---+---+---+---+
 */
setup_idt:
	lea ignore_int,%edx
	movl $0x00080000,%eax
	movw %dx,%ax		/* selector = 0x0008 = cs */
	movw $0x8E00,%dx	/* interrupt gate - dpl=0, present */

	lea idt,%edi
	mov $256,%ecx
rp_sidt:
	movl %eax,(%edi)
	movl %edx,4(%edi)
	addl $8,%edi
	dec %ecx
	jne rp_sidt
    lidtl idt_descr
    ret

    .section .kernel_end
kernel_end_flag:
	    .4byte  0xdeadbeef
