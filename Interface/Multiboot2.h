/*=++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * File: Multiboot2.h
 *
 * Summary:
 * This file provides the interface for loading kernels with the Multiboot2 protocol
 * and parsing the Multiboot structures.
 *
 * Copyright (C) 2017 - Shukant Pal
 *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=*/
#ifndef __MULTIBOOT2_H__
#define __MULTIBOOT2_H__

#include <TYPE.h>

#define MULTIBOOT_SEARCH					32768
#define MULTIBOOT_HEADER_ALIGN			8

#define MULTIBOOT2_HEADER_MAGIC			0xE85250D6 // BootService::multiboot2HeaderMagic
#define MULTIBOOT2_BOOTLOADER_MAGIC		0x36D76289 // IA32::EAX

#define MULTIBOOT_MOD_ALIGN				0x00001000
#define MULTIBOOT_INFO_ALIGN				0x00000008

#define MULTIBOOT_TAG_ALIGN                  8
#define MULTIBOOT_TAG_TYPE_END               0
#define MULTIBOOT_TAG_TYPE_CMDLINE          	1
#define MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME  2
#define MULTIBOOT_TAG_TYPE_MODULE            3
#define MULTIBOOT_TAG_TYPE_BASIC_MEMINFO     4
#define MULTIBOOT_TAG_TYPE_BOOTDEV          	5
#define MULTIBOOT_TAG_TYPE_MMAP             	6
#define MULTIBOOT_TAG_TYPE_VBE   			7
#define MULTIBOOT_TAG_TYPE_FRAMEBUFFER      	8
#define MULTIBOOT_TAG_TYPE_ELF_SECTIONS 	9
#define MULTIBOOT_TAG_TYPE_APM        	 	10
#define MULTIBOOT_TAG_TYPE_EFI32      		11
#define MULTIBOOT_TAG_TYPE_EFI64			12
#define MULTIBOOT_TAG_TYPE_SMBIOS			13
#define MULTIBOOT_TAG_TYPE_ACPI_OLD  		14
#define MULTIBOOT_TAG_TYPE_ACPI_NEW		15
#define MULTIBOOT_TAG_TYPE_NETWORK 		16

#define MULTIBOOT_HEADER_TAG_END_TYPE  				0
#define MULTIBOOT_HEADER_TAG_INFORMATION_REQUEST_TYPE  1
#define MULTIBOOT_HEADER_TAG_ADDRESS_TYPE  			2
#define MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS_TYPE  		3
#define MULTIBOOT_HEADER_TAG_CONSOLE_FLAGS_TYPE  		4
#define MULTIBOOT_HEADER_TAG_FRAMEBUFFER_TYPE  		5
#define MULTIBOOT_HEADER_TAG_MODULE_ALIGN_TYPE  		6

#define MULTIBOOT_ARCHITECTURE_I386  		0
#define MULTIBOOT_ARCHITECTURE_MIPS32		4
#define MULTIBOOT_HEADER_TAG_OPTIONAL 		1

#define MULTIBOOT_CONSOLE_FLAGS_CONSOLE_REQUIRED 		1
#define MULTIBOOT_CONSOLE_FLAGS_EGA_TEXT_SUPPORTED 	2

typedef
struct _MULTIBOOT_HEADER {
	U32 MagicNo; /* Must equal MULTIBOOT_HEADER_MAGIC */
	U32 Architecture; /* ISA Architecture - 0 for x86 */
	U32 HeaderLength; /* Length of the header, along with optional tags */
	U32 Checksum; /* Sum of this and the above fields must equal 0 */
} MULTIBOOT_HEADER;

typedef
struct _MULTIBOOT_HEADER_TAG {
	U16 Type;
	U16 Flags;
	U32 Size;
} MULTIBOOT_HEADER_TAG;

typedef
struct _MULTIBOOT_HEADER_TAG_INFORMATION_REQUEST {
	U16 Type;
	U16 Flags;
	U32 Size;
	U32 Requests[0];
} MULTIBOOT_HEADER_TAG_INFORMATION_REQUEST;

typedef
struct _MULTIBOOT_HEADER_TAG_ADDRESS {
	U16 Type;
	U16 Flags;
	U32 Size;
	U32 HeaderAddress;
	U32 LoadStartAddress;
	U32 LoadEndAddress;
	U32 BSSEndAddress;
} MULTIBOOT_HEADER_TAG_ADDRESS;

typedef
struct _MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS {
	U16 Type;
	U16 Flags;
	U32 Size;
	U32 EntryAddress;
} MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS;

typedef
struct _MULTIBOOT_HEADER_TAG_CONSOLE_FLAGS {
	U16 Type;
	U16 Flags;
	U32 Size;
	U32 ConsoleFlags;
} MULTIBOOT_HEADER_TAG_CONSOLE_FLAGS;

typedef
struct _MULTIBOOT_HEADER_TAG_FRAME_BUFFER {
	U16 Type;
	U16 Flags;
	U32 Size;
	U32 Width;
	U32 Height;
	U32 Depth;
} MULTIBOOT_HEADER_TAG_FRAME_BUFFER;

typedef
struct _MULTIBOOT_HEADER_TAG_MODULE_ALIGN {
	U16 Type;
	U16 Flags;
	U32 Size;
} MULTIBOOT_HEADER_TAG_MODULE_ALIGN;

typedef
struct _MULTIBOOT_HEADER_TAG_RELOCATABLE {
	U16 Type;
	U16 Flags;
	U32 Size;
	U32 MinimumAddress;
	U32 MaximumAddress;
	U32 Align;
	U32 Preference;
} MULTIBOOT_HEADER_TAG_RELOCATABLE;

typedef
struct _MULTIBOOT_COLOR {
	U8 Red;
	U8 Green;
	U8 Blue;
} MULTIBOOT_COLOR;

#define MULTIBOOT_MEMORY_AVAILABLE 1
#define MULTIBOOT_MEMORY_RESERVED 2
#define MULTIBOOT_MEMORY_ACPI_RECLAIMABLE 3
#define MULTIBOOT_MEMORY_NVS 4
#define MULTIBOOT_MEMORY_BADRAM 5

/* CircuitKernel Memory Type ------------------- */
#define MULTIBOOT_MEMORY_MODULE 0xDEE100FE
#define MULTIBOOT_MEMORY_STRUCT 0xDEE200FE

/**
 * Type: MULTIBOOT_MMAP_ENTRY
 *
 * Summary:
 * This represents a physical memory-region entry in the Multiboot2 memory
 * map. Its size is specified in the MULTIBOOT_MMAP.EntrySize field.
 */
typedef
struct _MULTIBOOT_MMAP_ENTRY {
	U64 Address;/* Physical address of the physical-memory region*/
	U64 Length;/* Length/size of the physical-memory region */
	U32 Type;/* Type of the memory region */
	U32 Zero;/* Reserved field, which should have a value of zero */
} MULTIBOOT_MMAP_ENTRY;

typedef
struct _MULTIBOOT_TAG {
	U32 Type;
	U32 Size;
} MULTIBOOT_TAG;

typedef
struct _MULTIBOOT_TAG_STRING {
	U32 Type;
	U32 Size;
	CHAR String[0];
} MULTIBOOT_TAG_STRING;

/**
 * MULTIBOOT_TAG_MODULE -
 *
 * Summary:
 * This type is used to referring to multiboot2 'module', loaded by the
 * bootloader. This tag may appear multiple times and the function
 * SearchMultipleMultibootTags() should be used to get the nth module by
 * tag index.
 *
 * @Version 1
 * @Since Circuit 2.03
 * @Author Shukant Pal
 * @See Multiboot 1.6
 */
typedef
struct _MULTIBOOT_TAG_MODULE {
	U32 Type;/* must equal MULTIBOOT_TAG_TYPE_MODULE */
	U32 Size;/* Size of the module descriptor, including CMDLine */
	U32 ModuleStart;/* Physical load-address of the module */
	U32 ModuleEnd;/* Physical address of the end-of-module */
	CHAR CMDLine[0];/* Text associated with the module at load-time */
} MULTIBOOT_TAG_MODULE;

/**
 * MULTIBOOT_TAG_BASIC_MEMINFO - 
 *
 * Summary:
 * This type is used to get information on physical memory present
 * in the system at boot-time.
 *
 * @Version 1
 * @Since Circuit 2.03
 * @Author Shukant Pal
 * @See Multiboot 1.6
 */
typedef
struct _MULTIBOOT_TAG_BASIC_MEMINFO {
	U32 Type;/* must equal MULTIBOOT_TAG_TYPE_BASIC_MEMINFO */
	U32 Size;/* must equal 16 */
	U32 MmLower;/* Lower-memory present at boot-time */
	U32 MmUpper;/* High-memory present at boot-time */
} MULTIBOOT_TAG_BASIC_MEMINFO;

typedef
struct _MULTIBOOT_TAG_BOOTDEV {
	U32 Type;
	U32 Size;
	U32 BIOSDevice;
	U32 Slice;
	U32 Part;
} MULTIBOOT_TAG_BOOTDEV;

/**
 * MULTIBOOT_TAG_MMAP -
 *
 * Summary:
 * This type contains the initial data for retrieving the memory map
 * from the boot loader, which consists of entries, to load the PMA.
 *
 * Fields:
 * Type - must equal MULTIBOOT_TAG_TYPE_MMAP
 * Size - must equal 16
 * EntrySize - Size of each entries, so that further versions can be made
 * EntryVersion - Version of the entries, but is backward-compatible
 *
 * @Version 1
 * @Since Circuit 2.03
 * @Author Shukant Pal
 * @See Multiboot1.6
 */
typedef
struct _MULTIBOOT_TAG_MMAP {
	U32 Type;
	U32 Size;
	U32 EntrySize;
	U32 EntryVersion;
	/* Entries are present just after this */
} MULTIBOOT_TAG_MMAP;

typedef
struct _MULTIBOOT_VBE_INFO_BLOCK {
	U8 ExternalSpecification[512];
} MULTIBOOT_VBE_INFO_BLOCK;

typedef
struct _MULTIBOOT_VBE_MODE_INFO_BLOCK {
	U8 ExternalSpecification[256];
} MULTIBOOT_VBE_MODE_INFO_BLOCK;

typedef
struct _MULTIBOOT_TAG_VBE {
	U32 Type;
	U32 Size;
	U16 VBEMode;
	U16 VBEInterfaceSegment;
	U16 VBEInterfaceOffset;
	U16 VBEInterfaceLength;
	MULTIBOOT_VBE_INFO_BLOCK ControlInfo;
	MULTIBOOT_VBE_MODE_INFO_BLOCK ModeInfo;
} MULTIBOOT_TAG_VBE;

#define MULTIBOOT_FRAMEBUFFER_TYPE_INDEXED 0
#define MULTIBOOT_FRAMEBUFFER_TYPE_RGB 1
#define MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT 2

typedef struct _MULTIBOOT_TAG_FRAMEBUFFER_COMMON {
	U32 Type;
	U32 Size;
	U64 Address;
	U32 Pitch;
	U32 Width;
	U32 Height;
	U8 BPP;
	U8 BufferType;
	U16 Reserved;
} MULTIBOOT_TAG_FRAMEBUFFER_COMMON;

typedef struct _MULTIBOOT_TAG_FRAMEBUFFER {
	MULTIBOOT_TAG_FRAMEBUFFER_COMMON Common;
	union {
		struct {
			U16 PaletteColors;
			MULTIBOOT_COLOR Palette[0];
		};
		struct {
			U8 RedFieldPosition;
			U8 RedMaskSize;
			U8 GreenFieldPosition;
			U8 GreenMaskSize;
			U8 BlueFieldPosition;
			U8 BlueMaskSize;
		};
	};
} MULTIBOOT_TAG_FRAMEBUFFER;

/**
 * Type: MULTIBOOT_TAG_ELF_SECTIONS
 *
 * Summary:
 * This type contains the kernel ELF-section table. As the ELF header needn't be
 * present after the bootloader loads the kernel into memory, the multiboot specs
 * have provided a way to get the 'section-header-table'.
 *
 * @See Module/ELF.h - For more information, on section-headers
 * @See Module/MSI.h - This tag is the basis of the 'Multiboot Section Interface'
 * @Version 1
 * @Since Circuit 2.03
 * @Author Shukant Pal
 */
typedef struct _MULTIBOOT_TAG_ELF_SECTIONS {
	U32 Type;/* equ 9 */
	U32 Size;
	U32 Number;/* Number of sections present in the kernel */
	U32 EntrySize;/* Size of each section-header-entry */
	U32 ShstrIndex;/* Index of the section containing string table for all section names */
	CHAR Sections[0];/* Section-header-table raw data */
} MULTIBOOT_TAG_ELF_SECTIONS;

typedef struct _MULTIBOOT_TAG_APM {
	U32 Type;
	U32 Size;
	U16 Version;	
	U16 CSegment;
	U16 Offset;
	U16 CSegment16;
	U16 DSegment;
	U16 Flags;
	U16 CSegmentLength;
	U16 CSegment16Length;
	U16 DSegmentLength;
} MULTIBOOT_TAG_APM;

typedef
struct _MULTIBOOT_TAG_EFI32 {
	U32 Type;
	U32 Size;
	U32 Pointer;
} MULTIBOOT_TAG_EFI32;

typedef
struct _MULTIBOOT_TAG_EFI64 {
	U32 Type;
	U32 Size;
	U64 Pointer;
} MULTIBOOT_TAG_EFI64;

typedef
struct _MULTIBOOT_TAG_SMBIOS {
	U32 Type;
	U32 Size;
	U8 Major;
	U8 Minor;
	U8 Reserved[6];
	U8 Tables[0];
} MULTIBOOT_TAG_SMBIOS;

typedef
struct _MULTIBOOT_TAG_OLD_ACPI {
	U32 Type;
	U32 Size;
	U8 RSDP[0];
} MULTIBOOT_TAG_OLD_ACPI;

typedef
struct _MULTIBOOT_TAG_NEW_ACPI {
	U32 Type;
	U32 Size;
	U8 RSDP[0];
} MULTIBOOT_TAG_NEW_ACPI;

typedef
struct _MULTIBOOT_TAG_NETWORK {
	U32 Type;
	U32 Size;
	U8 DHCPack[0];
} MULTIBOOT_TAG_NETWORK;

/**
 * SearchMultibootTag() - 
 *
 * Summary:
 * This function is used for searching a tag with a given a type, after
 * loading the multiboot tags.
 *
 * Args:
 * tagType - Type field of the tag
 *
 * @Version 1
 * @Since Circuit 2.03
 * @Author Shukant Pal
 * @See Multiboot 1.6
 */
VOID *SearchMultibootTag(
	U32 tagType
);

/**
 * Multiboot_Tag_Search_From() -
 *
 * Summary:
 * This function is used for searching the nth occurence of a given tag,
 * after loading the multiboot tags.
 *
 * Args:
 * lastTag - Tag from it should search
 * tagType - Type of the tag
 *
 * @Version 1
 * @Since Circuit 2.03
 * @Author Shukant Pal
 * @See Multiboot 1.6
 */
VOID *SearchMultibootTagFrom(
	VOID *lastTag,
	U32 tagType
);

#endif /* Multiboot2.h */
