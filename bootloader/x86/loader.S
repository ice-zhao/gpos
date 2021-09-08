    /* loader kernel and get machine data using BIOS routines */
	.code16
	.section .text
    .include "load.inc"
    .include "pm.inc"

   	jmp _start
    .include "fat12hdr.inc"

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
    .4byte (LOADER_PHY_ADDR + LABEL_GDT)    /* GDT base */

    /* GDT selector */
    .equ 	SelectorFlatC, (LABEL_DESC_FLAT_C	- LABEL_GDT)
    .equ	SelectorFlatRW, (LABEL_DESC_FLAT_RW	- LABEL_GDT)
    .equ	SelectorVideo, (LABEL_DESC_VIDEO	- LABEL_GDT + SA_RPL3)


_start:
    mov %cs, %ax
    mov %ax, %ds
    mov %ax, %es
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

/* Find KERNEL.BIN in root directory of driver 0 */
    movw    $SecNoOfRootDir, (wSectorNo)    /* 19 */

/* Read root dir sector to memory */
LABEL_SEARCH_IN_ROOT_DIR_BEGIN:
    cmpw    $0,(wRootDirSizeForLoop)    /* If searching in root dir 14*/
    jz      LABEL_NO_LOADERBIN          /* can find  LOADER.BIN ?   */
    decw    (wRootDirSizeForLoop)
    mov     $BaseOfLoader,%ax
    mov     %ax,%es                     /* %es <- BaseOfKernel*/
    mov     $0,%bx                      /* %bx <- Offset 0 */
    mov     (wSectorNo),%ax             /* %ax <- sector number in root */
    mov     $1,%cl
    call    ReadSector
    mov     $KernelFileName,%si         /* %ds:%si -> kernel  BIN */
    mov     $0,%di                      /* BaseOfLoader<<4+0*/
    cld
    mov     $0x10,%dx   /* 16 directory entry/sector */

/* Search for "KERNEL  BIN", FAT12 save file name in 11 bytes, 8 bytes for
   file name, 3 bytes for suffix. If file name is less than 8 bytes, filled with '\20'.
   So "KERNEL.BIN" is saved as: "KERNEL  BIN".
*/
LABEL_SEARCH_FOR_LOADERBIN:
    cmp     $0,%dx                      /* Read control */
    jz      LABEL_GOTO_NEXT_SECTOR_IN_ROOT_DIR
    dec     %dx
    mov     $11,%cx

LABEL_CMP_FILENAME:
    cmp     $0,%cx
    jz      LABEL_FILENAME_FOUND    /* If 11 chars are all identical? */
    dec     %cx
    lodsb                           /* %ds:(%si) -> %al*/
    cmp     %es:(%di),%al
    jz      LABEL_GO_ON
    jmp     LABEL_DIFFERENT         /* Different */

LABEL_GO_ON:
    inc     %di
    jmp     LABEL_CMP_FILENAME      /* Go on loop */

LABEL_DIFFERENT:
    and     $0xffe0,%di             /* Go to head of this entry */
    add     $0x20,%di
    mov     $KernelFileName,%si     /* Next entry */
    jmp     LABEL_SEARCH_FOR_LOADERBIN

LABEL_GOTO_NEXT_SECTOR_IN_ROOT_DIR:
    addw    $1,(wSectorNo)
    jmp     LABEL_SEARCH_IN_ROOT_DIR_BEGIN

	/* Not found kernel.BIN in root dir. */
LABEL_NO_LOADERBIN:
    mov     $3,%dh
    call    DispStrRealMode         /* Display string(index 3) */
    jmp        .            /* Infinite loop */

/* Found. */
LABEL_FILENAME_FOUND:
    mov     $RootDirSectors,%ax     /* 14 */
    and     $0xffe0,%di             /* Start of current entry, 32 bytes per entry */
    add     $0x1a,%di               /* First sector of this file */
    mov     %es:(%di),%cx
    push    %cx                     /* Save index of this sector in FAT */
    add     %ax,%cx
    add     $DeltaSecNo,%cx         /* KERNEL.BIN's start sector saved in %cl */ /* 17 */
    movw    $BaseOfKernel,%ax
    mov     %ax,%es                 /* %es <- BaseOfKernel */
    mov     $OffsetOfKernel,%bx     /* %bx <- OffsetOfKernel */
    mov     %cx,%ax                 /* %ax <- Sector number */

/* Load KERNEL.BIN's sector's to memory. */
LABEL_GOON_LOADING_FILE:
    push    %ax
    push    %bx
    mov     $0x0e,%ah
    mov     $'.',%al    /* Char to print */
    mov     $0x0f,%bl   /* Front color: white */
    int     $0x10       /* BIOS int 10h, ah=0xe: Print char */
    pop     %bx
    pop     %ax

    mov     $1,%cl
    call    ReadSector
    pop     %ax             /* Got index of this sector in FAT */
    call    GetFATEntry
    cmp     $0x0fff,%ax
    jz      LABEL_FILE_LOADED
    push    %ax             /* Save index of this sector in FAT */
    mov     $RootDirSectors,%dx
    add     %dx,%ax
    add     $DeltaSecNo,%ax
    add     (BPB_BytsPerSec),%bx
    jmp     LABEL_GOON_LOADING_FILE

LABEL_FILE_LOADED:
    mov     $2,%dh                  #display ready
    call   	DispStrRealMode        /* Display string(index 1) */

#get current cursor position and save it
	mov	$BaseOfLoader, %ax	#base address of machine data
	mov	%ax, %ds
	mov	$0x03, %ah	# read cursor pos
	xor	%bh, %bh
	int	$0x10		# save it in known place, con_init fetches
	mov	%dx, %ds:0	# it from 0x90000.

# Get memory size (extended mem, kB) this only work for 286 and 386
#	mov	$0x88, %ah
#	int	$0x15
#	mov	%ax, %ds:2

#Get memory size (KB)
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

# Get video-card data:
	mov	$0x0f, %ah
	int	$0x10
	mov	%bx, %ds:4	# bh = display page
	mov	%ax, %ds:6	# al = video mode, ah = window width

# check for EGA/VGA and some config parameters
	mov	$0x12, %ah
	mov	$0x10, %bl
	int	$0x10
	mov	%ax, %ds:8
	mov	%bx, %ds:10
	mov	%cx, %ds:12


# Get hd0 data
    push %ds
    push %es
    mov	$0x0000, %ax
	mov	%ax, %ds
	lds	%ds:4*0x41, %si
	mov	$BaseOfLoader, %ax
	mov	%ax, %es
	mov	$0x0080, %di
	mov	$0x10, %cx
	rep
	movsb

# Get hd1 data
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

/*move kernel to address 0x0*/
    push %es
    push %ds
    cli
	mov	$0x0000, %ax
	cld			# 'direction'=0, movs moves forward
do_move:
	mov	%ax, %es	# destination segment
	add	$0x1000, %ax
	cmp	$0x9000, %ax
	jz	end_move
	mov	%ax, %ds	# source segment
	sub	%di, %di
	sub	%si, %si
	mov $0x8000, %cx
	rep
	movsw
	jmp	do_move
end_move:
    pop %ds
    pop %es

    #enter protected mode
    lgdtl GdtPtr

    /* enable A20 */
    call FAST_ENABLE_A20LINE

    /* set protected mode bit */
    movl %cr0, %eax
    orl  $1, %eax
    movl %eax, %cr0

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

/*******************************************************************
   Jump to KERNEL.BIN's start address in memory.
*/
    ljmpl     $SelectorFlatC,$(OffsetOfKernel)
#    ljmpl     $SelectorFlatC,$(LOADER_PHY_ADDR + LABEL_TEST)
/*******************************************************************/

    .code32
    .section .text
LABEL_TEST:
    mov $SelectorVideo, %ax
    mov %ax, %gs
    movl $((80 * 11 + 0) * 2), %edi
    movb $0xC, %ah /* 0000: Black Back 1100: Red Front */
    movb $'L', %al
    mov %ax, %gs:(%edi)

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
   Routine: GetFATEntry
   Action: Find %ax sector's index in FAT, save result in %ax
*/
GetFATEntry:
    push    %es
    push    %bx
    push    %ax
    mov     $BaseOfLoader,%ax
    sub     $0x0100,%ax
    mov     %ax,%es           /* Left 4K bytes for FAT */
    pop     %ax
    mov     $12,%bx
    mul     %bx               /* %dx:%ax = %ax*12bit */
    mov     $8,%bx
    div     %bx               /* %dx:%ax/8bit */
    movb    %dl, (bOdd)       /* store remainder %dx in label bOdd. */

LABEL_EVEN:
    xor     %dx,%dx           /* Now %ax is the offset of FATEntry in FAT */
    mov     (BPB_BytsPerSec),%bx
    div     %bx               /* %dx:%ax/BPB_BytsPerSec */
    push    %dx
    mov     $0,%bx
    add     $SecNoOfFAT1,%ax  /* %ax <- FATEntry's sector */
    mov     $2,%cl            /* Read 2 sectors in 1 time, because FATEntry */
    call    ReadSector        /* may be in 2 sectors. */
    pop     %dx
    add     %dx,%bx
    mov     %es:(%bx),%ax     /* read FAT entry by word(2 bytes) */
    cmpb    $0,(bOdd)         /* remainder %dx(see above) == 0 ?*/
    jz      LABEL_EVEN_2      /* NOTE: %ah: high address byte, %al: low byte */
    shr     $4,%ax

LABEL_EVEN_2:
    and     $0x0fff,%ax

LABEL_GET_FAT_ENTRY_OK:
    pop     %bx
    pop     %es
    ret


/* ==================================================================
   Routine: ReadSector
   Action: Read %cl Sectors from %ax sector(floppy) to %es:%bx(memory)
     Assume sector number is 'x', then:
       x/(BPB_SecPerTrk) = y,
       x%(BPB_SecPerTrk) = z.
     The remainder 'z' PLUS 1 is the start sector number;
     The quotient 'y' divide by BPB_NumHeads(RIGHT SHIFT 1 bit)is cylinder
       number;
     AND 'y' by 1 can got magnetic header.
*/
ReadSector:
    push    %ebp
    mov     %esp,%ebp
    sub     $2,%esp        /* Reserve space for saving %cl */
    mov     %cl,-2(%ebp)
    push    %bx            /* Save bx */
    mov     (BPB_SecPerTrk), %bl    /* %bl: the divider */
    div     %bl            /* 'y' in %al, 'z' in %ah */
    inc     %ah            /* z++, got start sector */
    mov     %ah,%cl        /* %cl <- start sector number */
    mov     %al,%dh        /* %dh <- 'y' */
    shr     $1,%al         /* 'y'/BPB_NumHeads */
    mov     %al,%ch        /* %ch <- Cylinder number(y>>1) */
    and     $1,%dh         /* %dh <- Magnetic header(y&1) */
    pop     %bx            /* Restore %bx */
    /* Now, we got cylinder number in %ch, start sector number in %cl, magnetic
       header in %dh. */
    mov     (BS_DrvNum), %dl
GoOnReading:
    mov     $2,%ah
    mov     -2(%ebp),%al    /* Read %al sectors */
    int     $0x13
    jc      GoOnReading     /* If CF set 1, mean read error, reread. */
    add     $2,%esp
    pop     %ebp
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
wRootDirSizeForLoop:    .2byte  RootDirSectors /* 14 */
wSectorNo:              .2byte  0       /* Sector number to read */
bOdd:                   .byte   0       /* odd or even? */
_dwMCRNumber:           .4byte  0
_MemChkBuf:             .fill 256, 1, 0
