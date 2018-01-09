/*=++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * File: linker.ld (/linker.ld - Core)
 *
 * Summary:
 * Linker script for (GNU-ld/Link compat) linkers for the CIRCUIT-KTERM code.
 *
 * Copyright (C) 2017 - Shukant Pal
 *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=*/
 
OUTPUT_FORMAT("elf32-i386")
ENTRY(KModuleMain)
 
PHDRS {
	KmTextSegment PT_LOAD FILEHDR;/* Text segment will be read-only, execute */
	KmDataSegment PT_LOAD;/* Data segment will be read-write, no-execute (for security) */
	KmDynamicSegment PT_DYNAMIC;
}

SECTIONS {
	. = 0x00000000 + SIZEOF_HEADERS;
	
	.text ALIGN(0x1000) : {
		ctorsStart = .;
		*(SORT(.ctors*))
		ctorsEnd = .;
		
		dtorsStart = .;
		*(SORT(.dtors*))
		dtorsEnd = .;
		
		*(.text)
		*(.text*)
		*(.rdata)
		*(.rodata)
	} :KmTextSegment
	
	.data ALIGN(0x1000):{
		*(.data)
		*(.COMMON)
		*(.bss)
	} :KmDataSegment

	.dynamic ALIGN(0x1000):{
		*(.dynamic)
	} :KmDynamicSegment
	
	/DISCARD/ :
	{
		*(.interp)
		*(.gcc_except_table)
		*(.eh_frame)
		*(.note)
		*(.comment)
	}
} 