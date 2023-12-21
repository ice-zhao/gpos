    .code16	    #using 16bit code segment
    .section .text
    .include "load.inc"
    jmp     LABEL_START /* Start to boot. */
    /* Initial registers. */
_start:
LABEL_START:
    cli
    mov     %cs,%ax
    mov     %ax,%ds
    mov     %ax,%es
    mov     %ax,%ss
    mov     $BaseOfStack, %sp

    /* Clear screen */
    mov     $0x0600,%ax   /* %ah=6, %al=0 */
    mov     $0x0700,%bx   /* Black white */
    mov     $0,%cx        /* Top left: (0,0) */
    mov     $0x184f,%dx   /* Bottom right: (80,50) */
    int     $0x10         /* BIOS int 10h, ah=6: Initialize screen */

    /* Display "Booting**" */
    mov     $0,%dh
    call    DispStr       /* Display string(index 0)*/

    call  check_int13_hard_disk_extension

    /*load hdloader.bin from hard disk, this file start from 1 sector.(LBA) */
    call load_hdloader

    /*now, hdloader.bin is in memory 0000:0500. ready to run*/
    jmp $0,$0x500

load_hdloader:
    mov $disk_address_packet_struct, %si
    mov $0x42, %ah
    mov $0x80, %dl
    int $0x13
    jc  read_sector_error
    ret

/* ==================================================================
   Routine: DispStr
   Action: Display a string, string index stored in %dh
*/
DispStr:
    mov     $MessageLength, %ax
    mul     %dh
    add     $BootMessage,%ax
    mov     %ax,%bp               /* String address */
    mov     %ds,%ax
    mov     %ax,%es
    mov     $MessageLength,%cx    /* String length */
    mov     $0x1301,%ax           /* ah = 0x13, al = 0x01(W) */
    mov     $0x07,%bx             /* PageNum 0(bh = 0), bw(bl= 0x07)*/
    mov     $0,%dl                /* Start row and column */
    int     $0x10                 /* BIOS INT 10h, display string */
    ret

check_int13_hard_disk_extension:
    mov $0x41, %ah
    mov $0x55AA, %bx
    mov $0x80, %dl
    int $0x13
    jc  HD_not_support_int13_extension
    ret

HD_not_support_int13_extension:
    mov     $4,%dh
    call    DispStr       /* Display string(index 4)*/
    jmp .

read_sector_error:
    mov     $3,%dh
    call    DispStr       /* Display string(index 3)*/
    jmp .


    .section .data
    .align 4
disk_address_packet_struct:
	    .byte  0x10     /* disk address packet structure size */
	    .byte  0        /* reserved, must be 0 */
        .byte  16       /* the number of sectors to be loaded */ /* 16*512B=8KB */
        .byte  0        /* reserved, must be 0 */
	    .2byte 0x500    /* buffer offset*/
	    .2byte 0        /* buffer segment */
	    .4byte 1        /* LBA low part of start addr (sector number, based 0)*/
	    .4byte 0        /* LBA high part of start addr */

/* ==================================================================
   String table
*/
.set    MessageLength,15
BootMessage:        .ascii    "Booting********"        /* index 0 */
Message1:           .ascii    "Loaded in      "        /* index 1 */
Message2:           .ascii    "No LOADER      "        /* index 2 */
Message3:           .ascii    "ReadSectorError"        /* index 3 */
Message4:           .ascii    "No HD Extension"        /* index 4 */
