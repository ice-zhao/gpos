#where loader is loaded
.equ    BaseOfStack,     0x7c00    /* Stack base address, inner */
.equ    BaseOfLoader,     0x9000
.equ    OffsetOfLoader,     0x400
.equ    LOADER_PHY_ADDR,    (BaseOfLoader * 0x10)
.equ    BaseOfKernel,       0x1000
.equ    OffsetOfKernel,     0x0

