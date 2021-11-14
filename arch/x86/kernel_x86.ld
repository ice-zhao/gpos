SECTIONS
{
    . = 0x0;
    _start_kernel = .;

    .text :
    {
        _ftext = .;
    }

    .data : ALIGN(8)
    {
        _data = .;
        *(.data)
        *(.data.*)
        _edata = .;
    }

    .bss : {
        _bss = . ;
        *(.bss)
		*(.bss.*)
		*(COMMON)
        _ebss = . ;
    }

    .kernel_end :
    {
    }

    _end_kernel = ALIGN(4k);
    _pg_dir = _end_kernel;
    /*dynamic allocation page structure at the end of kernel*/
}
