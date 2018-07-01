/**
 * @file Multiboot2.h
 *
 * -------------------------------------------------------------------
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 *
 * Copyright (C) 2017 - Shukant Pal
 */
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

struct MultibootHeaderTagFrameBuffer
{
	U16 Type;
	U16 Flags;
	U32 Size;
	U32 Width;
	U32 Height;
	U32 Depth;
};

struct MultibootHeaderTagModuleAlign {
	U16 type;
	U16 flags;
	U32 size;
};

struct MultibootHeaderTagRelocatable
{
	U16 type;
	U16 flags;
	U32 size;
	U32 minimumAddress;
	U32 maximumAddress;
	U32 align;
	U32 preference;
};

struct MultibootColor {
	U8 red;
	U8 green;
	U8 blue;
};

#define MULTIBOOT_MEMORY_AVAILABLE 1
#define MULTIBOOT_MEMORY_RESERVED 2
#define MULTIBOOT_MEMORY_ACPI_RECLAIMABLE 3
#define MULTIBOOT_MEMORY_NVS 4
#define MULTIBOOT_MEMORY_BADRAM 5

/* CircuitKernel Memory Type ------------------- */
#define MULTIBOOT_MEMORY_MODULE 0xDEE100FE
#define MULTIBOOT_MEMORY_STRUCT 0xDEE200FE

struct MultibootMMapEntry {
	U64 address;/* Physical address of the physical-memory region*/
	U64 length;/* Length/size of the physical-memory region */
	U32 type;/* Type of the memory region */
	U32 zero;/* Reserved field, which should have a value of zero */

#ifndef _CBUILD
	inline MultibootMMapEntry *next(unsigned long entrySize) {
		return ((MultibootMMapEntry *)((char *) this + entrySize));
	}

	inline MultibootMMapEntry *prev(unsigned long entrySize) {
		return ((MultibootMMapEntry *)((char *) this + entrySize));
	}
#endif
};

union MultibootMMapSearch {
	struct MultibootMMapEntry *ent;
	unsigned int entPtr;
};

typedef
struct MultibootTag {
	U32 type;
	U32 size;
} MULTIBOOT_TAG;

typedef
struct MultibootTagString {
	U32 Type;
	U32 Size;
	char String[0];
} MULTIBOOT_TAG_STRING;

struct MultibootTagModule
{
	U32 type;
	U32 size;
	U32 moduleStart;
	U32 moduleEnd;
	const char CMD_Line[0];
};

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
struct MultibootTagBasicMemInfo {
	U32 Type;/* must equal MULTIBOOT_TAG_TYPE_BASIC_MEMINFO */
	U32 Size;/* must equal 16 */
	U32 MmLower;/* Lower-memory present at boot-time */
	U32 MmUpper;/* High-memory present at boot-time */
} MULTIBOOT_TAG_BASIC_MEMINFO;

typedef
struct MultibootTagBootDev {
	U32 Type;
	U32 Size;
	U32 BIOSDevice;
	U32 Slice;
	U32 Part;
} MULTIBOOT_TAG_BOOTDEV;

typedef
struct MultibootTagMMap {
	U32 type;
	U32 size;
	U32 entrySize;
	U32 entryVersion;
	/* Entries are present just after this */

#ifndef _CBUILD
	inline MultibootMMapEntry *getEntries() {
		return ((MultibootMMapEntry*)(this + 1));
	}

	inline bool inBounds(MultibootMMapEntry *entry) {
		return ((char *) entry < (char *) this + size);
	}
#endif
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
			struct MultibootColor Palette[0];
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
	char Sections[0];/* Section-header-table raw data */
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

union MultibootSearch
{
	struct MultibootTag *tag;
	struct MultibootTagString *str;
	struct MultibootTagModule *mod;
	struct MultibootTagBasicMemInfo *memInfo;
	struct MultibootTagBootDev *bootDevice;
	struct MultibootTagMMap *mmap;
	unsigned long loc;

#ifndef _CBUILD
	MultibootSearch() {

	}

	MultibootSearch(MultibootSearch &copyFrom) {
		this->tag = copyFrom.tag;
	}

	MultibootSearch(MultibootTag *tag) {
		this->tag = tag;
	}

	MultibootSearch(MultibootTagModule *mod) {
		this->mod = mod;
	}
#endif
};

#ifndef _CBUILD

class MultibootChannel
{
public:
	static inline unsigned long getMultibootTableSize() {
		return (tagFence - (unsigned long) tagTable + 8);
	}

	static inline unsigned long getMultibootTableSize(MultibootTag *table) {
		return (*(unsigned long*) table);
	}

	static void init(unsigned long physicalAddress);
	static MultibootSearch getTag(U32 typeToken);
	static MultibootSearch getNextTagOfType(MultibootSearch from);

	static inline MultibootTagString *getBootloaderName() {
		return (getTag(MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME).str);
	}

	static inline MultibootTagBasicMemInfo *getBasicMemInfo() {
		return (getTag(MULTIBOOT_TAG_TYPE_BASIC_MEMINFO).memInfo);
	}

	static inline MultibootTagBootDev *getBootDevice() {
		return (getTag(MULTIBOOT_TAG_TYPE_BOOTDEV).bootDevice);
	}

	static inline MultibootTagModule *getFirstModule() {
		return (getTag(MULTIBOOT_TAG_TYPE_MODULE).mod);
	}

	static inline MultibootTag *getFirstTag() {
		return (tagTable);
	}

	static inline MultibootTagMMap *getMMap() {
		return (getTag(MULTIBOOT_TAG_TYPE_MMAP).mmap);
	}

	static inline MultibootTagModule *getNextModule(MultibootTagModule *mod) {
		MultibootSearch mods(mod);
		return (getNextTagOfType(mods).mod);
	}

	static inline void getNextTag(MultibootSearch &gptr) {
		gptr.loc += (gptr.tag->size % 8) ?
				(gptr.tag->size & ~7) + 8 :
				(gptr.tag->size);
	}

private:
	static MultibootTag *tagTable;
	static unsigned long tagFence;
	MultibootChannel();
};

#endif
#endif /* Multiboot2.h */
