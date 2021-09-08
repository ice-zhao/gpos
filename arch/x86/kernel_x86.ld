SECTIONS
{
    . = 0x0;
    _start_kernel = .;
    .text :
    {
        _ftext = .;
    } = 0

    .data :
    {
    } = 0

    .kernel_end :
    {
    } = 0

    _end_kernel=ALIGN(4k);
    /*dynamic allocation page structure at the end of kernel*/
}
