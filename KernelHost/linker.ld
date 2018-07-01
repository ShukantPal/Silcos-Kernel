/* @file linker.ld
 
 This is the linker script for the KernelHost, which is the only position
 independent executable in the kernel-environment. It can be located anywhere
 in physical memory, depending where the initializer decides to put it.
 
 Most likely, the kernel will be located exactly at MARK_16M (16-MB address),
 just after DMA memory.
 
 No relocations are performed while copying the segments of the
 executable. The ModuleLoader logic is present in the KernelHost itself, and
 therefore some bootstrapping mechansim is done in it. The initializer only
 and only moves the segments of the executable.
 
 ------------------------------------------------------------------------------
 */

OUTPUT_FORMAT("elf32-i386")
ENTRY(HostEntry)

KERNEL_OFFSET = 0xC0000000;
KERNEL_START = 0xC0100000;

PHDRS {
	kernel PT_LOAD FILEHDR;
	dynamic PT_DYNAMIC;
}

SECTIONS {
	. = SIZEOF_HEADERS;
	KernelStart = .;

	KernelCodeStart = .;
	.text :
	{		
		ctorsStart = .;
		*(SORT(.ctors*))
		ctorsEnd = .;
				
		*(.init)
		*(.fini)
		*(.text)
		*(.text.*)
	} 
	KernelCodeEnd = .;

	.data ALIGN (0x1000) :
	{
	KernelDataStart = .;
		*(COMMON)
		*(.data)
		*(.rdata)
		*(.rodata*)
	KernelDataEnd = .;
	
	KernelBSSStart = .;
		*(.bss)
	KernelBSSEnd = .;
	
	KernelPDatStart = .;
	*(.KernelPermData)
	KernelPDatEnd = .;
	}	
	
	.dynamic :
	{
				msiKernelDynamicTableStart = .;
		*(.dynamic)
				msiKernelDynamicTableEnd = .;
	} :kernel :dynamic
	
	/DISCARD/ :
    {
        *(.comment)
        *(.eh_frame) /* discard this, unless you are implementing runtime support for C++ exceptions. */
        *(.dtors*) /* Kernel isn't stupid to destroy global objects */
    }

	KernelEnd = .;
	HALData = .;
}

