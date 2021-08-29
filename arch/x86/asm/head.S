	.code16
	.section .text
    .global idt, gdt
	jmp LABEL_START
    .include "realmode.inc"
    .include "pm.inc"

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

_cursor_pos:
    .4byte (80 * 7 + 0) * 2   # at 7th line, 0 column
    .equ   cursor_pos, (KERNEL_PHY_ADDR + _cursor_pos)



_start:
LABEL_START:
    mov %cs, %ax
    mov %ax, %ds
    mov %ax, %es
    mov $BaseOfStack, %ax
    mov %ax, %ss
    mov $0, %sp

    /* Clear screen */
    mov     $0x0600,%ax   /* %ah=6, %al=0 */
    mov     $0x0700,%bx   /* Black white */
    mov     $0,%cx        /* Top left: (0,0) */
    mov     $0x184f,%dx   /* Bottom right: (80,50) */
    int     $0x10         /* BIOS int 10h, ah=6: Initialize screen */

    /* show kernel string */
    mov $0, %dh
    call DispStrRealMode

    cli
#reprogram 8259A, hardware interrupt from int 0x20 - int 0x2F
	mov	$0x11, %al		# initialization sequence(ICW1)
					# ICW4 needed(1),CASCADE mode,Level-triggered
	out	%al, $0x20		# send it to 8259A-1
	.word	0x00eb,0x00eb		# jmp $+2, jmp $+2
	out	%al, $0xA0		# and to 8259A-2
	.word	0x00eb,0x00eb
	mov	$0x20, %al		# start of hardware int's (0x20)(ICW2)
	out	%al, $0x21		# from 0x20-0x27
	.word	0x00eb,0x00eb
	mov	$0x28, %al		# start of hardware int's 2 (0x28)
	out	%al, $0xA1		# from 0x28-0x2F
	.word	0x00eb,0x00eb		#               IR 7654 3210
	mov	$0x04, %al		# 8259-1 is master(0000 0100) --\
	out	%al, $0x21		#				|
	.word	0x00eb,0x00eb		#			 INT	/
	mov	$0x02, %al		# 8259-2 is slave(       010 --> 2)
	out	%al, $0xA1
	.word	0x00eb,0x00eb
	mov	$0x01, %al		# 8086 mode for both
	out	%al, $0x21
	.word	0x00eb,0x00eb
	out	%al, $0xA1
	.word	0x00eb,0x00eb
	mov	$0xFF, %al		# mask off all interrupts for now
	out	%al, $0x21
	.word	0x00eb,0x00eb
	out	%al, $0xA1

    /* enter protected mode */
    lgdtl GdtPtr
	call setup_idt

    /* enable A20 */
    call FAST_ENABLE_A20LINE

    /* set protected mode bit */
    movl %cr0, %eax
    orl  $1, %eax
    movl %eax, %cr0

    /* jump to protected mode */
    ljmpl $SelectorFlatC, $(KERNEL_PHY_ADDR+start_kernel)

start_kernel:
	.code32
	.section .text
    .global iprintk, start_kernel,end_kernel
    /* init protected mode segments environment */
	mov $SelectorFlatRW, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
    mov %ax, %ss
    movl $0x90000, %esp

    mov $(SelectorVideo), %ax
    mov %ax, %gs /* Video segment selector(dest) */

    pushl $int_msg
    call iprintk
    add $4, %esp

    movl $((80 * 11 + 0) * 2), %edi
    movb $0xC, %ah /* 0000: Black Back 1100: Red Front */
    movb $'K', %al
    mov %ax, %gs:(%edi)

	 /* Stop here, infinite loop. */
	jmp .

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
	popl %eax
	pop %fs
	pop %es
	pop %ds
	popl %edx
	popl %ecx
	popl %eax
	iret


/*
    void iprintk(char* str)
parameter:
    str:  the pointer of string to output
*/
iprintk:
    pushl %ebp
    movl %esp, %ebp
    pushl %ebx
    pushl %esi
    pushl %edi

    movl 8(%ebp), %esi
    movl $cursor_pos, %eax
    movl (%eax), %edi       #load cursor position
    movb $0xF, %ah          #charactor color

.1:
    lodsb
    test %al, %al       #get string's end char 0?
    jz .2
    cmp $0xA, %al       #is an enter?
    jnz .3
    pushl %eax
    movl %edi, %eax
    mov $160, %bl
    div %bl
    and $0xFF, %eax
    inc %eax
    mov $160, %bl
    mul %bl
    movl %eax, %edi
    popl %eax
    jmp .1
.3:
    movw %ax, %gs:(%edi)
    add $2, %edi
    jmp .1
.2:
	movl $cursor_pos, %eax
    movl %edi, (%eax)        #save cursor current position

    popl %edi
    popl %esi
    popl %ebx
    popl %ebp
    ret

    #32bit code end

    .code16
FAST_ENABLE_A20LINE:
    push %ax
	inb $0x92, %al
    orb $0b00000010, %al
    outb %al, $0x92
    pop %ax
    ret

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



/* ==================================================================
   String table
*/
.equ    MessageLength,   9
LoadMessage:        .ascii    "Kernel   "        /* index 0 */
Message1:           .ascii    "         "        /* index 1 */
Message2:           .ascii    "	Ready.  "        /* index 2 */
Message3:           .ascii    "No KERNEL"        /* index 3 */
Message4:           .ascii    "Too Large"        /* index 4 */

/* ==================================================================
   Routine: DispStrRealMode
   Action: Display a string, string index stored in %dh
*/
DispStrRealMode:
    mov     $MessageLength, %ax
    mul     %dh
    add     $LoadMessage,%ax
    mov     %ax,%bp               /* String address */
    mov     %ds,%ax
    mov     %ax,%es               /* ES:BP= string address */
    mov     $MessageLength,%cx    /* String length */
    mov     $0x1301,%ax           /* ah = 0x13, al = 0x01(W) */
    mov     $0x07,%bx             /* PageNum 0(bh = 0), bw(bl= 0x07)*/
    mov     $0,%dl                /* Start row and column */
    int     $0x10                 /* BIOS INT 10h, display string */
    ret

end_kernel:
	    .4byte  0xdeadbeef
