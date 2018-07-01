///
/// @file ELF.h
///
/// Contains all the data structures required by the ELF ABI to conform
/// to it.
/// -------------------------------------------------------------------
/// This program is free software: you can redistribute it and/or modify
/// it under the terms of the GNU General Public License as published by
/// the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program.  If not, see <http://www.gnu.org/licenses/>
///
/// Copyright (C) 2017 - Shukant Pal
///

#ifndef KERNHOST_MODULE_ELF_H
#define KERNHOST_MODULE_ELF_H

#include <TYPE.h>
#include <Utils/LinkedList.h>

#ifndef _CBUILD
namespace Module
{
#endif

struct ModuleRecord;

#ifndef _CBUILD
namespace Elf
{
#endif

#define EI_NIDENT 16

#ifdef ARCH_32
	typedef U32 ELF32_ADDR;
	typedef U16 ELF32_HALF;
	typedef U32 ELF32_OFF;
	typedef S32 ELF32_SWORD;
	typedef U32 ELF32_WORD;
#else
	// Not checked yet
	typedef U64 ELF64_ADDR;
	typedef U32 ELF64_HALF;
	typedef U64 ELF64_OFF;
	typedef S64 ELF64_SWORD;
	typedef U64 ELF32_WORD;
#endif

struct ElfHeader;
struct ElfProgramHeader;
struct ElfSectionHeader;

enum ElfClass
{
	ELFCLASSNONE=0,
	ELFCLASS32 = 1,
	ELFCLASS64 = 2
};

enum ElfData
{
	ELFDATANONE = 0,
	ELFDATA2LSB = 1,
	ELFDATA2MSB = 2
};

enum ElfType
{
	ET_NONE		= 0,// No file type
	ET_REL		= 1,// Relocatable file
	ET_EXEC		= 2,// Executable file
	ET_DYN		= 3,// Shared-object file
	ET_CORE		= 4,// Core file
	ET_LOPROC	= 0xFF00,// Processor-specific
	ET_HIPROC	= 0xFFFF// Processor-specific
};

enum ElfMachine
{
	EM_NONE	= 0,// No machine
	EM_M32	= 1,// AT&T WE 32100
	EM_SPARC= 2,// SPARC
	EM_386	= 3,// Intel 80386
	EM_68K	= 4,// Motorola 68000
	EM_88K	= 5,// Motorola 88000
	EM_860	= 7,// Intel 80860
	EM_MIPS	= 8// MIPS RS3000
};

#ifdef IA32
	#define EM_RUNNER EM_386
#endif

enum ElfVersion {
	EV_NONE	= 0, // Invalid Version
	EV_CURRENT = 1 // Current Version
};

struct ElfHeader
{
	#define EI_MAG0		0
	#define EI_MAG1		1
	#define EI_MAG2		2
	#define EI_MAG3		3
	#define EI_CLASS	4
	#define EI_DATA		5
	#define EI_VERSION	6
	#define EI_PAD		7
	#define EI_NIDENT	16

	#define ELFMAG0		0x7F
	#define ELFMAG1		'E'
	#define ELFMAG2		'L'
	#define ELFMAG3		'F'

	unsigned char fileIdentifier[EI_NIDENT];
	ELF32_HALF fileType;
	ELF32_HALF platformRequired;
	ELF32_WORD buildVersion;
	ELF32_ADDR entryAddress;
	ELF32_OFF programHeaderOffset;
	ELF32_OFF sectionHeaderOffset;
	ELF32_WORD machineFlags;
	ELF32_HALF headerSize;
	ELF32_HALF programHeaderEntrySize;
	ELF32_HALF programHeaderEntryCount;
	ELF32_HALF sectionHeaderEntrySize;
	ELF32_HALF sectionHeaderEntryCount;
	ELF32_HALF sectionStringIndex;
};

//! Gives the program-header table ptr associated with the ELF-header
#define PROGRAM_HEADER(eHeader)((ProgramHeader *)((unsigned long) eHeader \
		+ eHeader->programHeaderOffset))

//! Gives the section-header table ptr associated with the ELF-header
#define SECTION_HEADER(eHeader) (eHeader->sectionHeaderOffset) ? \
		((SectionHeader *)((unsigned long) eHeader + \
				eHeader->sectionHeaderOffset)) : NULL
///
/// Special section-table header index that have special meaning and
/// may not really exist.
///
/// @version 1.0
/// @since Circuit 2.03
/// @author Shukant Pal
///
enum SectionIndex
{
	SHN_UNDEF 	= 0,//!< Undefined section-header reference
	SHN_LORESERVE	= 0xFF00,
	SHN_LOPROC	= 0xFF00,
	SHN_HIPROC	= 0xFF1F,
	SHN_ABS		= 0xFFF1,
	SHN_COMMON	= 0xFFF2,//!< Specifies the symbol as COMMON
	SHN_HIRESERVCE	= 0xFFFF,
};

enum SectionType
{
	SHT_NULL	= 0,
	SHT_PROGBITS	= 1,
	SHT_SYMTAB	= 2,
	SHT_STRTAB	= 3,
	SHT_RELA	= 4,
	SHT_HASH	= 5,
	SHT_DYNAMIC	= 6,
	SHT_NOTE	= 7,
	SHT_NOBITS	= 8,
	SHT_REL		= 9,
	SHT_SHLIB	= 10,
	SHT_DYNSYM	= 11,
	SHT_LOPROC	= 0x70000000,
	SHT_HIPROC	= 0x7FFFFFFF,
	SHT_LOUSER	= 0x80000000,
	SHT_HIUSER	= 0xFFFFFFFF
};

enum SectionFlag
{
	SHF_WRITE	= 0x1,
	SHF_ALLOC	= 0x2,
	SHF_EXECINSTR	= 0x4,
	SHF_MASKPROC	= 0xF0000000
};

struct SectionHeader
{
	ELF32_WORD Name;
	ELF32_WORD Type;
	ELF32_WORD Flags;
	ELF32_ADDR Address;
	ELF32_OFF Offset;
	ELF32_WORD Size;
	ELF32_WORD Link;
	ELF32_WORD Info;
	ELF32_WORD AddressAlign;
	ELF32_WORD EntrySize;
};

#define STN_UNDEF			0
#define ELF32_ST_BIND(value) ((value) >> 4)

enum SymbolBind
{
	STB_LOCAL  = 0,
	STB_GLOBAL = 1,
	STB_WEAK   = 2,
	STB_LOPROC = 13,
	STB_HIPROC = 15
};

#define ELF32_ST_TYPE(value) ((value) & 0xF)

enum SymbolType
{
	STT_NOTYPE	= 0,
	STT_OBJECT	= 1,
	STT_FUNC	= 2,
	STT_SECTION	= 3,
	STT_FILE	= 4,
	STT_LOPROC	= 13,
	STT_HIPROC	= 15
};

#define ELF32_ST_INFO(b, t) (((b) << 4) + ((t)&0xF))

struct Symbol
{
	ELF32_WORD name;
	ELF32_ADDR value;
	ELF32_WORD size;
	unsigned char info;
	unsigned char other;
	ELF32_HALF sectionIndex;
};

enum RelocationType
{
	R_386_NONE	= 0,
	R_386_32	= 1,
	R_386_PC32	= 2,
	R_386_GOT32	= 3,
	R_386_PLT32	= 4,
	R_386_COPY	= 5,
	R_386_GLOB_DAT	= 6,
	R_386_JMP_SLOT	= 7,
	R_386_RELATIVE	= 8,
	R_386_GOTOFF	= 9,
	R_386_GOTPC	= 10
};

#define ELF32_R_SYM(i) ((i) >> 8)
#define ELF32_R_TYPE(i) ((unsigned char) (i))
#define ELF32_R_INFO(s, t) (((s) << 8) + (unsigned char) (t))

struct RelEntry
{
	ELF32_ADDR offset;
	ELF32_WORD info;
};

struct RelaEntry
{
	ELF32_ADDR offset;
	ELF32_WORD info;
	ELF32_SWORD addend;
};

enum PhdrType
{
	PT_NULL		= 0,
	PT_LOAD		= 1,
	PT_DYNAMIC	= 2,
	PT_INTERP	= 3,
	PT_NOTE		= 4,
	PT_SHLTB	= 5,
	PT_PHDR		= 6,
	PT_LOPROC	= 0x70000000,
	PT_HIPROC	= 0x7FFFFFFF
};

struct ProgramHeader
{
	ELF32_WORD entryType;
	ELF32_OFF fileOffset;
	ELF32_ADDR virtualAddress;
	ELF32_ADDR physicalAddress;
	ELF32_WORD fileSize;
	ELF32_WORD memorySize;
	ELF32_WORD flagSet;
	ELF32_WORD alignBoundary;
};

enum DynamicTag
{
	DT_NULL 	= 0,
	DT_NEEDED 	= 1,
	DT_PLTRELSZ	= 2,
	DT_PLTGOT	= 3,
	DT_HASH		= 4,
	DT_STRTAB	= 5,
	DT_SYMTAB	= 6,
	DT_RELA		= 7,
	DT_RELASZ	= 8,
	DT_RELAENT	= 9,
	DT_STRSZ	= 10,
	DT_SYMENT	= 11,
	DT_INIT		= 12,
	DT_FINI		= 13,
	DT_SONAME	= 14,
	DT_RPATH	= 15,
	DT_SYMBOLIC	= 16,
	DT_REL		= 17,
	DT_RELSZ	= 18,
	DT_RELENT	= 19,
	DT_PLTREL	= 20,
	DT_DEBUG	= 21,
	DT_TEXTREL	= 22,
	DT_JMPREL	= 23,
	DT_INIT_ARRAY	= 25,
	DT_FINI_ARRAY	= 26,
	DT_INIT_ARRAYSZ	= 27,
	DT_FINI_ARRAYSZ	= 28,
	DT_RUNPATH	= 29,
	DT_FLAGS	= 30,
	DT_ENCODING	= 32,
	DT_PREINIT_ARRAY= 32,
	DT_PREINIT_ARRAYSZ= 33,
	DT_MAXPOSTAGS	= 34,
	DT_LOPROC	= 0x70000000,
	DT_HIPROC	= 0x7FFFFFFF
};

struct DynamicEntry
{
	ELF32_SWORD tag;
	union
	{
		ELF32_WORD val;
		ELF32_ADDR ptr;
	};
};

struct SymbolTable
{
	char *nameTable;
	struct Symbol *entryTable;
	unsigned long entryCount;
};

struct HashTable
{
	struct SectionHeader *hashSectionHdr;
	unsigned long bucketEntries;
	unsigned long chainEntries;
	unsigned long *bucketTable;
	unsigned long *chainTable;
};

struct RelaTable
{
	struct RelaEntry *entryTable;
	unsigned long entryCount;
	unsigned long entrySize;
};

struct RelTable
{
	struct RelEntry *entryTable;
	unsigned long entryCount;
	unsigned long entrySize;
};

struct RelocationTable
{
	union
	{
		struct RelaEntry *relaEntries;
		struct RelEntry *relEntries;
		unsigned long tableLocation;
	};
	unsigned long entryCount;
	unsigned long entrySize;
	unsigned long relocType;
};

struct DynamicTable {
	struct ElfDynamicEntry *EntryTable;
	unsigned long EntryCount;
};

struct ProgramCache {
	unsigned long PageCount;/* Total number of pages */
	struct ProgramHeader *Dynamic;
	struct DynamicTable DynamicTable;
};

#ifndef _CBUILD
}// namespace Elf
#endif

#ifndef _CBUILD
}// namespace Module
#endif

#endif/* Module/ELF.h */
