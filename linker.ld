/*=++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * File: linker.ld (/linker.ld - Core)
 *
 * Summary:
 * GNU linker script
 *
 * Copyright (C) 2016, 2017 - Shukant Pal
 *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=*/

OUTPUT_FORMAT("elf32-i386")
ENTRY(InitRuntime)

SECTIONS {
	. = 0xc0100000 + SIZEOF_HEADERS;
	KernelStart = .;

	KernelCodeStart = .;
	.text ALIGN (0x1000) : AT(ADDR(.text) - 0xC0000000)
	{
		*(.text)
		*(.rdata)
		*(.rodata)
	}
	KernelCodeEnd = .;


	KernelDataStart = .;
	.data ALIGN (0x1000) : AT(ADDR(.data) - 0xC0000000)
	{
		*(.data)
	}
	KernelDataEnd = .;

	KernelBSSStart = .;
	.bss ALIGN (0x1000) : AT(ADDR(.bss) - 0xC0000000)
	{
		*(COMMON)
		*(.bss)
	}
	KernelBSSEnd = .;

	KernelEnd = .;
	HALData = .;
}
