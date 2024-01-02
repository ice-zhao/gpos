    /* loader kernel and get machine data using BIOS routines */
	.code16
	.section .text
    .include "load.inc"
    .include "pm.inc"
    .include "hdregs.inc"

   	jmp _start
    .align 8
/* GDT ------------------------------------------------------------------------------------------------------------------------------------------------------------
                                            段基址            段界    , 属性 */
gdt:
LABEL_GDT:              Descriptor             0,                    0, 0						/* 空描述符 */
LABEL_DESC_FLAT_C:		Descriptor             0,             0x0fffff, (DA_CR  | DA_32 | DA_LIMIT_4K)			/* 0 ~ 4G */
LABEL_DESC_FLAT_RW:		Descriptor             0,             0x0fffff, (DA_DRW | DA_32 | DA_LIMIT_4K)			/* 0 ~ 4G */
LABEL_DESC_VIDEO:		Descriptor         0x0B8000,           0x0ffff, (DA_DRW | DA_DPL3)	/* 显存首地址 */
/* GDT ------------------------------------------------------------------------------------------------------------------------------------------------------------*/
    .equ	GdtLen, (. - LABEL_GDT) /* GDT length */
GdtPtr:
    .2byte (GdtLen - 1) /* GDT limit */
    .4byte (LABEL_GDT)    /* GDT base */

    /* GDT selector */
    .equ 	SelectorFlatC, (LABEL_DESC_FLAT_C	- LABEL_GDT)
    .equ	SelectorFlatRW, (LABEL_DESC_FLAT_RW	- LABEL_GDT)
    .equ	SelectorVideo, (LABEL_DESC_VIDEO	- LABEL_GDT + SA_RPL3)


_start:
    mov %cs, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %ss
    mov $0x7c00, %sp

    /* Clear screen */
    call clear_screen

    /* show loading string */
    mov $0, %dh
    call DispStrRealMode

    /* change %ds to 0x9000, get current cursor position and save it */
	mov	$BaseOfLoader, %ax	#base address of machine data
	mov	%ax, %ds
	mov	$0x03, %ah	# read cursor pos
	xor	%bh, %bh    # video page
	int	$0x10		# save it in known place, con_init fetches
	mov	%dx, %ds:0	# it from 0x90000.

    /* Get video-card data: */
	mov	$0x0f, %ah
	int	$0x10
	mov	%bx, %ds:4	# bh = display page
	mov	%ax, %ds:6	# al = video mode, ah = window width

/*
   	BH = 0 if color mode in effect
	   = 1 if mono mode in effect
	BL = 0 if 64k EGA memory
	   = 1 if 128k EGA memory
	   = 2 if 192k EGA memory
	   = 3 if 256k EGA memory
*/
    /* check for EGA/VGA and some config parameters */
	mov	$0x12, %ah
	mov	$0x10, %bl
	int	$0x10
	mov	%ax, %ds:8
	mov	%bx, %ds:10
	mov	%cx, %ds:12

    /* Get hd0 data */
    push %ds
    push %es
    mov	$0x0000, %ax
	mov	%ax, %ds
    xorl %esi, %esi
    /* interrupt vector number: 0x41. every vector occupy 4Bytes.
       4*0x41: vector 0x41's address
       load this address's content to %si, %ds
       use %ds:%si as the start address to read hard disk parameters.
    */
	lds	%ds:4*0x41, %si
	mov	$BaseOfLoader, %ax
	mov	%ax, %es
	mov	$0x0080, %di
	mov	$0x10, %cx
	rep
	movsb

    /* Get hd1 data */
	mov	$0x0000, %ax
	mov	%ax, %ds
	lds	%ds:4*0x46, %si
	mov	$BaseOfLoader, %ax
	mov	%ax, %es
	mov	$0x0090, %di
	mov	$0x10, %cx
	rep
	movsb
    pop %es
    pop %ds

    call clear_screen

    /* restore %ds to default value */
    mov %cs, %ax
    mov %ax, %ds

    /*Get memory size (KB) */
    movl $0, %ebx           #first call sets 0, BIOS fills value after later call
    movw $_MemChkBuf, %di   #es:di pointers to (Address Range Descriptor Structure)
.MemChkLoop:
    movl $0xE820, %eax
    mov  $20, %cx    #	Address Range Descriptor Structure's size
    movl $0x534D4150, %edx  #signature 'SMAP', BIOS uses signature to verify this call
    int $0x15
	jc	.MemChkFail
    add $20, %di
    incl (_dwMCRNumber)
    cmp $0, %ebx        #Contains the continuation value to get the next address descriptor
    jne .MemChkLoop
    jmp .MemChkOk
.MemChkFail:
.MemChkOk:
    movl $_dwMCRNumber, %esi
    movl (_dwMCRNumber), %ecx

    /* enter protected mode */
    lgdtl GdtPtr

    /* enable A20 */
    call FAST_ENABLE_A20LINE

    /* set protected mode bit */
    movl %cr0, %eax
    orl  $1, %eax
    movl %eax, %cr0

    /* reprogram 8259A, hardware interrupt from int 0x20 - int 0x2F */
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

/*******************************************************************
    */
    ljmpl     $SelectorFlatC, $LABEL_START
/*******************************************************************/

    .code32
    .section .text
LABEL_START:
    mov $SelectorFlatRW, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %ss
    mov $0x90000, %esp

    mov $SelectorVideo, %ax
    mov %ax, %gs

    movl $((80 * 10 + 0) * 2), %edi
    movb $0xC, %ah /* 0000: Black Back 1100: Red Front */
    movb $'L', %al
    mov %ax, %gs:(%edi)

    /* show memory layout of installed. */
    call DispMemInfo

    call main

	/* jump to kernel */
	ljmpl     $SelectorFlatC, $KERNEL_START_ADDRESS

DispMemInfo:
    pushl %edx
    pushl %esi
    pushl %edi
    pushl %ecx

    pushl $szMemChkTitle
    call DispStr
    add $4, %esp

    movl $MemChkBuf, %esi
    movl (_dwMCRNumber), %ecx   #total of ARDS(Address Range Descriptor Structure)
.loop:
    mov $5, %edx    #every ARDS has 5 part
    movl $ARDStruct, %edi      #pointer to ARDS temp storage buffer
.part1:
    pushl (%esi)
    call DispInt

    popl %eax   #store to _ARDStruct, a temp storage buffer
    stosl
    add $4, %esi
    dec %edx
    cmp $0, %edx
    jnz .part1
	call	DispReturn		#printf("\n");
    cmp $1, (dwType)
    jne .part2
    movl (dwBaseAddrLow), %eax
    addl (dwLengthLow), %eax
    cmp (dwMemSize), %eax
    jb .part2
    movl %eax, (dwMemSize)
.part2:
    loop .loop

   	call	DispReturn		#printf("\n");
    pushl $szRAMSize
    call DispStr
    add $4, %esp

    pushl (dwMemSize)
    call DispInt
    add $4, %esp

    xor %edx, %edx          #store memory size to machine data area in KB unit
    movl (dwMemSize), %eax
    movl $1024, %ecx
    div %ecx
    movl %eax, (memory_size_addr)

    popl %ecx
    popl %edi
    popl %esi
    popl %edx
    ret


# ------------------------------------------------------------------------
# 换行
# ------------------------------------------------------------------------
DispReturn:
	push	$szReturn
	call	DispStr			#printf("\n");
    add $4, %esp
	ret
# DispReturn 结束---------------------------------------------------------

#display a string that end with 0
DispStr:
    push %ebp
    mov %esp, %ebp
    push %ebx
    push %esi
    push %edi

    mov 8(%ebp), %esi
    movl (cursor_pos), %edi
    mov $0x0F, %ah      #gray color
.1:
    lodsb
    test %al, %al
    jz .2
    cmp $0xA, %al       #carriage return
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
    movl %edi, (cursor_pos)

    pop %edi
    pop %esi
    pop %ebx
    pop %ebp
    ret

#show an integer number
	.globl DispInt
	.type DispInt, @function
DispInt:
    movl 4(%esp), %eax
    shr $24, %eax
    call DispAL

    movl 4(%esp), %eax
    shr $16, %eax
    call DispAL

    movl 4(%esp), %eax
    shr $8, %eax
    call DispAL

    movl 4(%esp), %eax
    call DispAL

    mov $0x07, %ah  #0000b: black background, 0111b gray char
    mov $'h', %al
    push %edi
    movl (cursor_pos), %edi
    movw %ax, %gs:(%edi)
    add $4, %edi
    movl %edi, (cursor_pos)
    pop %edi
    ret

#show %al's number
DispAL:
    pushl %ecx
    pushl %edx
    pushl %edi

    movl (cursor_pos), %edi
    mov $0x0F, %ah  #0000: black background, 1111b: white word
    mov %al, %dl
    shr $4, %al
    mov $2, %ecx
.begin:
    and $0x0F, %al
    cmp $9, %al
    ja .1st
    add $'0', %al
    jmp .2nd
.1st:
    sub $0xA, %al
    add $'A', %al
.2nd:
    movw %ax, %gs:(%edi)
    add $2, %edi

    mov %dl, %al
    loop .begin

    movl %edi, (cursor_pos)

    popl %edi
    popl %edx
    popl %ecx

    ret     #the end of DispAL

	/*int read_blocks(unsigned int block_no, unsigned int counts, char* buffer)*/
.globl read_blocks
.type read_blocks, @function
read_blocks:
	pushl %ebp
	movl %esp, %ebp
	pushl %edx
	pushl %ecx
	pushl %ebx

	movl $HD_NSECTOR, %edx
	movl 12(%ebp), %eax	/* counts */
	shl $1, %eax	/* 2 sectors/block */
	outb %al, %dx	/* output total sectors to read */

	movl 8(%ebp), %eax	/* block index to read */
	shll $1, %eax	/* convert to sectors */
	addl $base_sector, %eax /* add base sector to got start sector to read */
	movl $HD_SECTOR, %edx
	outb %al, %dx
	movl $HD_LCYL, %edx
	shrl $8, %eax
	outb %al, %dx
	movl $HD_HCYL, %edx
	shrl $8, %eax
	outb %al, %dx
	movl $HD_CURRENT, %edx
	shrl $8, %eax
	andl $0x0f, %eax
	orl  $0xe0, %eax
	outb %al, %dx	/* LBA mode, primary disk and high LBA number */

	movl $HD_STATUS, %edx
	movb $HD_READ_CMD, %al
	outb %al, %dx	/* send read command */

	movl $HD_STATUS, %edx
1:
	inb %dx, %al
	and $0x88, %al
	cmp $0x08, %al
	jnz 1b

	movl $256, %eax
	movl 12(%ebp), %ecx	/* counts */
	shl  %cl, %eax
	movl %eax, %ecx
	movl $HD_DATA, %edx
	movl 16(%ebp), %ebx
2:
	in %dx, %ax
	mov %ax, (%ebx)
	addl $2, %ebx
	decl %ecx
	jnz 2b


	popl %ebx
	popl %ecx
	popl %edx
	popl %ebp
	ret


    .section .data
_szMemChkTitle:		.asciz	"BaseAddrL BaseAddrH LengthLow LengthHigh  Type\n"
	.equ szMemChkTitle,	(HD_LOADER_PHY_ADDR + _szMemChkTitle)
_cursor_pos:
    .4byte (80 * 0 + 0) * 2   # at 0 line, 0 column
    .equ   cursor_pos, (HD_LOADER_PHY_ADDR + _cursor_pos)

#32bit code end

    .code16
FAST_ENABLE_A20LINE:
    push %ax
	inb $0x92, %al
    orb $0b00000010, %al
    outb %al, $0x92
    pop %ax
    ret

clear_screen:
	    mov     $0x0600,%ax   /* %ah=6, %al=0 */
	    mov     $0x0700,%bx   /* Black white */
	    mov     $0,%cx        /* Top left: (0,0) */
	    mov     $0x184f,%dx   /* Bottom right: (80,50) */
	    int     $0x10         /* BIOS int 10h, ah=6: Initialize screen */
        ret


/* ==================================================================
   String table
*/
KernelFileName:     .asciz  "KERNEL  BIN"        /* File name */
.equ    MessageLength,   9
LoadMessage:        .ascii    "Loading  "        /* index 0 */
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

/* ==================================================================
   Variable table
*/
    .align 8
_dwMCRNumber:           .4byte  0
	    	.equ 	dwMCRNumber,	HD_LOADER_PHY_ADDR + _dwMCRNumber

_MemChkBuf:             .fill 256, 1, 0
    	.equ 	MemChkBuf,	HD_LOADER_PHY_ADDR + _MemChkBuf

_ARDStruct:
   	_dwBaseAddrLow:		.4byte	0
	_dwBaseAddrHigh:	.4byte	0
	_dwLengthLow:		.4byte	0
	_dwLengthHigh:		.4byte	0
	_dwType:            .4byte	0
	.equ   	ARDStruct,	HD_LOADER_PHY_ADDR + _ARDStruct
   	.equ   	dwBaseAddrLow,	HD_LOADER_PHY_ADDR + _dwBaseAddrLow
	.equ   	dwBaseAddrHigh,	HD_LOADER_PHY_ADDR + _dwBaseAddrHigh
	.equ   	dwLengthLow,	HD_LOADER_PHY_ADDR + _dwLengthLow
	.equ   	dwLengthHigh,	HD_LOADER_PHY_ADDR + _dwLengthHigh
	.equ   	dwType,	HD_LOADER_PHY_ADDR + _dwType

_dwMemSize:			.4byte	0
	.equ   	dwMemSize,	HD_LOADER_PHY_ADDR + _dwMemSize
_szRAMSize:			.asciz	"RAM size: "
	.equ   	szRAMSize,	HD_LOADER_PHY_ADDR + _szRAMSize

_szReturn:			.asciz  "\n"
	.equ 	szReturn,	HD_LOADER_PHY_ADDR + _szReturn
