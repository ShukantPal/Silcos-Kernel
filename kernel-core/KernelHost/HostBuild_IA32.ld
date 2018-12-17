/* Script for -z combreloc: combine and sort reloc sections */
/* Copyright (C) 2014-2018 Free Software Foundation, Inc.
 Copying and distribution of this script, with or without modification,
 are permitted in any medium without royalty provided the copyright
 notice and this notice are preserved.*/

OUTPUT_FORMAT("elf32-i386")
OUTPUT_ARCH(i386)

SECTIONS {
	. = SIZEOF_HEADERS;

	.note.gnu.build-id : { *(.note.gnu.build-id) }
	.hash : { *(.hash) }
	.gnu.hash : { *(.gnu.hash) }
	.dynsym : { *(.dynsym) }
	.dynstr : { *(.dynstr) }
	.gnu.version : { *(.gnu.version) }
	.gnu.version_d : { *(.gnu.version_d) }
	.gnu.version_r : { *(.gnu.version_r) }
	
	.rel.dyn : {
 		*(.rel.init)
 		*(.rel.text .rel.text.* .rel.gnu.linkonce.t.*)
		*(.rel.fini)
		*(.rel.rodata .rel.rodata.* .rel.gnu.linkonce.r.*)
		*(.rel.data.rel.ro .rel.data.rel.ro.* .rel.gnu.linkonce.d.rel.ro.*)
		*(.rel.data .rel.data.* .rel.gnu.linkonce.d.*)
		*(.rel.tdata .rel.tdata.* .rel.gnu.linkonce.td.*)
		*(.rel.tbss .rel.tbss.* .rel.gnu.linkonce.tb.*)
		*(.rel.ctors)
		*(.rel.dtors)
		*(.rel.got)
		*(.rel.bss .rel.bss.* .rel.gnu.linkonce.b.*)
		*(.rel.ifunc)
	}
	
	.rel.plt :
	{
		*(.rel.plt)
		PROVIDE_HIDDEN (__rel_iplt_start = .);
		*(.rel.iplt)
		PROVIDE_HIDDEN (__rel_iplt_end = .);
	}
	.init :
	{
		KEEP (*(SORT_NONE(.init)))
	}

	.plt : { *(.plt) *(.iplt) }
	.plt.got : { *(.plt.got) }
	.plt.sec : { *(.plt.sec) }
	
	.text :
	{
		*(.text.unlikely .text.*_unlikely .text.unlikely.*)
		*(.text.exit .text.exit.*)
		*(.text.startup .text.startup.*)
		*(.text.hot .text.hot.*)
		*(.text .stub .text.* .gnu.linkonce.t.*)
		/* .gnu.warning sections are handled specially by elf32.em.*/
		*(.gnu.warning)
	}

	.fini :
	{
		KEEP (*(SORT_NONE(.fini)))
	}

	.preinit_array :
	{
		PROVIDE_HIDDEN (__preinit_array_start = .);
		KEEP (*(.preinit_array))
		PROVIDE_HIDDEN (__preinit_array_end = .);
	}

	.init_array :
	{
		PROVIDE_HIDDEN (__init_array_start = .);
		KEEP (*(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.ctors.*)))
		KEEP (*(.init_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .ctors))
		PROVIDE_HIDDEN (__init_array_end = .);
	}

	.fini_array :
	{
		PROVIDE_HIDDEN (__fini_array_start = .);
		KEEP (*(SORT_BY_INIT_PRIORITY(.fini_array.*) SORT_BY_INIT_PRIORITY(.dtors.*)))
		KEEP (*(.fini_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .dtors))
		PROVIDE_HIDDEN (__fini_array_end = .);
	}

	.ctors	:
	{
		/*
		 * gcc uses crtbegin.o to find the start of
		 * the constructors, so we make sure it is
		 * first.Because this is a wildcard, it
		 * doesn't matter if the user does not
		 * actually link against crtbegin.o; the
		 * linker won't look for a file to match a
		 * wildcard.The wildcard also means that it
		 * doesn't matter which directory crtbegin.o
		 * is in.
		 */
		KEEP (*crtbegin.o(.ctors))
		KEEP (*crtbegin?.o(.ctors))
	
		/* We don't want to include the .ctor section from
		 the crtend.o file until after the sorted ctors.
		 The .ctor section from the crtend file contains the
		 end of ctors marker and it must be last */
		KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .ctors))
		KEEP (*(SORT(.ctors.*)))
		KEEP (*(.ctors))
	}

	.dtors	:
	{
		KEEP (*crtbegin.o(.dtors))
		KEEP (*crtbegin?.o(.dtors))
		KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .dtors))
		KEEP (*(SORT(.dtors.*)))
		KEEP (*(.dtors))
	}
	
	.jcr	: { KEEP (*(.jcr)) }
	.data.rel.ro	: { *(.data.rel.ro.local* .gnu.linkonce.d.rel.ro.local.*) 	*(.data.rel.ro .data.rel.ro.* .gnu.linkonce.d.rel.ro.*) }
	.dynamic	: { *(.dynamic) }
	.got		: { *(.got) *(.igot) }

	.got.plt	: { *(.got.plt)*(.igot.plt) }
	.data		:
	{
		*(.data .data.* .gnu.linkonce.d.*)
		SORT(CONSTRUCTORS)
	}

	.data1	: { *(.data1) }
	_edata = .; PROVIDE (edata = .);
	. = .;
	__bss_start = .;
	.bss		:
	{
		*(.dynbss)
		*(.bss .bss.* .gnu.linkonce.b.*)
		*(COMMON)
		/* Align here to ensure that the .bss section occupies space up to
		_end.Align after .bss to ensure correct alignment even if the
		.bss section disappears because there are no input sections.
		FIXME: Why do we need it? When there is no .bss section, we don't
		pad the .data section.*/
		 . = ALIGN(. != 0 ? 32 / 8 : 1);
	}

}
