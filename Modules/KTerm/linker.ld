/*=++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * File: linker.ld (/linker.ld - Core)
 *
 * Summary:
 * GNU linker script
 *
 * Copyright (C) 2017 - Shukant Pal
 *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=*/
 
OUTPUT_FORMAT("elf32-i386")
ENTRY(KModuleMain)
 
SECTIONS {
	. = 0x00000000;
	
	.text ALIGN(0x1000) : AT(.)
	{
		*(.text)
		*(.rdata)
		*(.rodata)
	}
	
	.data ALIGN(0x1000) : AT(.)
	{
		*(.data)
	}
	
	.bss ALIGN(0x1000) : AT(.)
	{
		*(.COMMON)
		*(.bss)
	}
} 
