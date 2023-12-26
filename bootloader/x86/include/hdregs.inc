#Hd controller regs. Ref: IBM AT Bios-listing
.equ HD_DATA,		0x1f0	/* _CTL when writing */
.equ HD_ERROR,		0x1f1	/* see err-bits */
.equ HD_NSECTOR,	0x1f2	/* nr of sectors to read/write */
.equ HD_SECTOR,		0x1f3	/* starting sector */
.equ HD_LCYL,		0x1f4	/* starting cylinder */
.equ HD_HCYL,		0x1f5	/* high byte of starting cyl */
.equ HD_CURRENT,	0x1f6	/* 101dhhhh , d=drive, hhhh=head */
.equ HD_STATUS,		0x1f7	/* see status-bits as read, command register as write*/
.equ HD_PRECOMP,	HD_ERROR	/* same io address, read=error, write=precomp */
.equ HD_COMMAND,	HD_STATUS	/* same io address, read=status, write=cmd */

.equ HD_CMD,		0x3f6

.equ HD_READ_CMD,	0x20

.equ base_sector,	0x800	/* partition start sector: 2048 */
