/**
 * @file InitorBuild.ld
 *
 * This is the linker-script for the initializer. It maps the executable
 * in **physical memory** at the 1 MB mark. This allows symbols to be used
 * as is without any relocation.
 *
 * Since the initializer lives in physical memory, where no page protection
 * exists, there is no use of separating the code and data. Efforts are to
 * squeeze the executable's mapping into the smallest space in memory - so
 * there is only one segment which holds both the code and data sections.
 *
 * @author Shukant Pal
 */
OUTPUT_FORMAT("elf32-i386")
ENTRY("StartupEntry")

FileOffset = 0x10000;

PHDRS {
	AllInOne PT_LOAD FILEHDR;
}

SECTIONS {
	. = FileOffset + SIZEOF_HEADERS;
	FileStart = .;
	
	.MultibootHeader : AT(ADDR(.MultibootHeader))
	{
		*(.MultibootHeader)
	} :AllInOne
	
	.text : AT(ADDR(.text))
	{
		*(.text)
	} :AllInOne
	
	.data : AT(ADDR(.data))
	{
		*(.data)
		*(.bss)
	} :AllInOne
	
	/DISCARD/ :
	{
		*(.eh_frame)
		*(.comment)
	}
	
	FileEnd = .;
}
