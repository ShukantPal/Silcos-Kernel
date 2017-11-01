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

KERNEL_OFFSET = 0xC0000000;
KERNEL_START = 0xC0100000;

PHDRS {
	kernel PT_LOAD FILEHDR;
	dynamic PT_DYNAMIC;
}

SECTIONS {
	. = KERNEL_START + SIZEOF_HEADERS;
	KernelStart = .;
	
	.BsTransferCtl : AT(ADDR(.BsTransferCtl) - KERNEL_OFFSET)
	{
		*(.BsTransferCtl)
	} :kernel

	KernelCodeStart = .;
	.text ALIGN (0x1000) : AT(ADDR(.text) - KERNEL_OFFSET)
	{		
		ctorsStart = .;
		*(SORT(.ctors*))
		ctorsEnd = .;
		
		dtorsStart = .;
		*(SORT(.dtors*))
		dtorsEnd = .;
		
		*(.text)
		*(.text.*)
		*(.rdata)
		*(.rodata*)
		*(.gnu.linkonce.t.*)
		*(.gnu.linkonce.r.*)
	} 
	KernelCodeEnd = .;

	KernelDataStart = .;
	.data ALIGN (0x1000) : AT(ADDR(.data) - KERNEL_OFFSET){
		*(.data)
		*(.gnu.linkonce.d.*)
	}
	KernelDataEnd = .;

	KernelBSSStart = .;
	.bss ALIGN (0x1000) : AT(ADDR(.bss) - KERNEL_OFFSET)
	{
		*(COMMON)
		*(.bss)
		*(.gnu.linkonce.b.*)
	}
	KernelBSSEnd = .;

	.dynamic ALIGN(0x1000) : AT(ADDR(.dynamic) - KERNEL_OFFSET)
	{
		*(.dynamic)
	} :dynamic
	
	/DISCARD/ :
    {
        *(.comment)
        *(.eh_frame) /* discard this, unless you are implementing runtime support for C++ exceptions. */
    }

	KernelEnd = .;
	HALData = .;
}

