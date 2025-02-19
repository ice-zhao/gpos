SECTIONS
{
	_main = .;

	.text : ALIGN(8)
	{
		_text = .;
		*(.text)
		*(.text.*)
		_etext = .;
	}

	.data : ALIGN(8)
	{
		_data = .;
		*(.data)
		*(.data.*)
		_edata = .;
	}

	.bss : ALIGN(8)
	{
		_bss = .;
		*(.bss)
		*(.bss.*)
		_ebss = .;
	}

	_end = .;
}
