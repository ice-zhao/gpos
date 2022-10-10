#floppy bootloader: where loader is loaded
.equ    BaseOfStack,     0x7c00    /* Stack base address, inner */
.equ    BaseOfLoader,     0x9000
.equ    OffsetOfLoader,     0x400
.equ    LOADER_PHY_ADDR,    (BaseOfLoader * 0x10)
.equ    BaseOfKernel,       0x1000
.equ    OffsetOfKernel,     0x0


#hard disk bootloader
.equ    HD_LOADER_PHY_ADDR,    (0x0)

#machine data
.equ    memory_size_offset,    0x200
.equ    memory_size_addr,   (LOADER_PHY_ADDR + memory_size_offset)
