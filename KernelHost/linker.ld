/* @file linker.ld
 *
 * This is the linker script for the KernelHost position-independent
 * executable.
 *
 */

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
	kernelSeg = .;
		*(.BsTransferCtl)
	} :kernel

	KernelCodeStart = .;
	.text : AT(ADDR(.text) - KERNEL_OFFSET)
	{		
		ctorsStart = .;
		*(SORT(.ctors*))
		ctorsEnd = .;
				
		*(.text)
		*(.text.*)
		*(.gnu.linkonce.t.*)
		*(.gnu.linkonce.r.*)
	} 
	KernelCodeEnd = .;

	.data ALIGN (0x1000) : AT(ADDR(.data) - KERNEL_OFFSET)
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
		/**(.gnu.linkonce.d.*)*/
	}	
	
	.dynamic : AT(ADDR(.dynamic) - KERNEL_OFFSET)
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

