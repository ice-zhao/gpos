SECTIONS
{
    . = 0x7c00;
    .text :
    {
        _ftext = .;              /* load program here */
    } = 0                        /* gap uses 0 to fill */

    .data :
    {
        __data_section_start = .;
    }
}
