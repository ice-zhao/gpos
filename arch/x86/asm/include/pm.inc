/* GDT Descriptor Attributes
     DA_  : Descriptor Attribute
     D    : Data Segment
     C    : Code Segment
     S    : System Segment
     R    : Read-only
     RW   : Read/Write
     A    : Access */
.equ    DA_32,  0x4000  /* 32-bit segment */
.equ    DA_LIMIT_4K, 0x8000     /* 4K */

/* Descriptor privilege level */
.equ    DA_DPL0,  0x00  /* DPL = 0 */
.equ    DA_DPL1,  0x20  /* DPL = 1 */
.equ    DA_DPL2,  0x40  /* DPL = 2 */
.equ    DA_DPL3,  0x60  /* DPL = 3 */

/* GDT Code- and Data-Segment Types */
.equ    DA_DR,   0x90   /* Read-Only */
.equ    DA_DRW,  0x92   /* Read/Write */
.equ    DA_DRWA, 0x93   /* Read/Write, accessed */
.equ    DA_C,    0x98   /* Execute-Only */
.equ    DA_CR,   0x9A   /* Execute/Read */
.equ    DA_CCO,  0x9C   /* Execute-Only, conforming */
.equ    DA_CCOR, 0x9E   /* Execute/Read-Only, conforming */

/* GDT System-Segment and Gate-Descriptor Types */
.equ    DA_LDT,      0x82    /* LDT */
.equ    DA_TaskGate, 0x85    /* Task Gate */
.equ    DA_386TSS,   0x89    /* 32-bit TSS(Available) */
.equ    DA_386CGate, 0x8C    /* 32-bit Call Gate */
.equ    DA_386IGate, 0x8E    /* 32-bit Interrupt Gate */
.equ    DA_386TGate, 0x8F    /* 32-bit Trap Gate */

/* Selector Attributes */
.equ    SA_RPL0, 0
.equ    SA_RPL1, 1
.equ    SA_RPL2, 2
.equ    SA_RPL3, 3

/* TI(Table Indicator) */
.equ    SA_TIG,  0  /* read descriptor from GDT */
.equ    SA_TIL,  4  /* read descriptor from LDT */

/* Page Attributes */
.equ    PG_P,    1
.equ    PG_RWR,  0
.equ    PG_RWW,  2
.equ    PG_USS,  0
.equ    PG_USU,  4

/* MACROS */

/* Segment Descriptor data structure.
   Usage: Descriptor Base, Limit, Attr
    Base:  4byte 
    Limit: 4byte (low 20 bits available)
    Attr:  2byte (lower 4 bits of higher byte are always 0) */
.macro Descriptor Base, Limit, Attr
    .2byte  \Limit & 0xFFFF
    .2byte  \Base & 0xFFFF
    .byte   (\Base >> 16) & 0xFF
    .2byte  ((\Limit >> 8) & 0xF00) | (\Attr & 0xF0FF)
    .byte   (\Base >> 24) & 0xFF
.endm

/* Gate Descriptor data structure.
   Usage: Gate Selector, Offset, PCount, Attr
    Selector:  2byte
    Offset:    4byte
    PCount:    byte
    Attr:      byte */
.macro Gate  Selector, Offset, PCount, Attr
    .2byte     (\Offset & 0xFFFF)
    .2byte     \Selector
    .2byte     (\PCount & 0x1F) | ((\Attr << 8) & 0xFF00)
    .2byte     ((\Offset >> 16) & 0xFFFF)
.endm

/* Initialize descriptor.
   Usage: InitDesc SegLabel, SegDesc */
.macro InitDesc SegLabel, SegDesc
    xor     %eax, %eax
    mov     %cs, %ax
    shl     $4, %eax
    addl    $(\SegLabel), %eax
    movw    %ax, (\SegDesc + 2)
    shr     $16, %eax
    movb    %al, (\SegDesc + 4)
    movb    %ah, (\SegDesc + 7)
.endm
