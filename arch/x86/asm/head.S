	.code16
	.section .text
	jmp LABEL_START
    .include "realmode.inc"
    .include "pm.inc"

/* GDT ------------------------------------------------------------------------------------------------------------------------------------------------------------
                                            段基址            段界    , 属性 */
LABEL_GDT:              Descriptor             0,                    0, 0						/* 空描述符 */
LABEL_DESC_FLAT_C:		Descriptor             0,             0x0fffff, (DA_CR  | DA_32 | DA_LIMIT_4K)			/* 0 ~ 4G */
LABEL_DESC_FLAT_RW:		Descriptor             0,             0x0fffff, (DA_DRW | DA_32 | DA_LIMIT_4K)			/* 0 ~ 4G */
LABEL_DESC_VIDEO:		Descriptor         0x0B8000,           0x0ffff, (DA_DRW | DA_DPL3)	/* 显存首地址 */
/* GDT ------------------------------------------------------------------------------------------------------------------------------------------------------------*/
    .equ	GdtLen, (. - LABEL_GDT) /* GDT length */
GdtPtr:
    .2byte (GdtLen - 1) /* GDT limit */
    .4byte (KERNEL_PHY_ADDR + LABEL_GDT)    /* GDT base */

    /* GDT selector */
    .equ 	SelectorFlatC, (LABEL_DESC_FLAT_C	- LABEL_GDT)
    .equ	SelectorFlatRW, (LABEL_DESC_FLAT_RW	- LABEL_GDT)
    .equ	SelectorVideo, (LABEL_DESC_VIDEO	- LABEL_GDT + SA_RPL3)

    /*.equ	BaseOfStack, 0x100*/
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

    /* show loading string */
    mov $0, %dh
    call DispStrRealMode

    /* enter protected mode */
    lgdtw GdtPtr

    /* enable A20 */
    call FAST_ENABLE_A20LINE

    /* set protected mode bit */
    movl %cr0, %eax
    orl  $1, %eax
    movl %eax, %cr0

    /* init protected mode segments environment */
	mov $SelectorFlatRW, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %ss

    /* jump to protected mode */
    ljmpl $SelectorFlatC, $(KERNEL_PHY_ADDR+LABEL_SEG_CODE32)

LABEL_SEG_CODE32:
	.code32
	.section .text
    mov $(SelectorVideo), %ax
    mov %ax, %gs /* Video segment selector(dest) */

    movl $((80 * 11 + 0) * 2), %edi
    movb $0xC, %ah /* 0000: Black Back 1100: Red Front */
    movb $'K', %al

    mov %ax, %gs:(%edi)

	 /* Stop here, infinite loop. */
	jmp .

    .code16
FAST_ENABLE_A20LINE:
    push %ax
	inb $0x92, %al
    orb $0b00000010, %al
    outb %al, $0x92
    pop %ax
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
