.equ    BaseOfLoader,     0x9000
.equ    LOADER_PHY_ADDR,    (BaseOfLoader * 0x10)

#machine data
.equ    memory_size_offset,    0x200
.equ    memory_size_addr,   (LOADER_PHY_ADDR + memory_size_offset)
.equ    paging_frames_offset,   0x204
.equ    paging_frames,   (LOADER_PHY_ADDR + paging_frames_offset)
.equ    paging_addr_offset,     0x208
.equ    paging_addr,   (LOADER_PHY_ADDR + paging_addr_offset)
