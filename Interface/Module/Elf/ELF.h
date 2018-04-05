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

namespace Module
{

struct ModuleRecord;

namespace Elf
{

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

///
/// Holds the summarized information regarding an elf-object file. It
/// is present at the very beginning of the blob/file and should be
/// first attested by @code ElfAnalyzer::validateBinary().
///
/// @version 1.0
/// @since Circuit 2.03
/// @author Shukant Pal
///
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
	STB_LOCAL  = 0,//!< STB_LOCAL - not visible outside object-file
	STB_GLOBAL = 1,//!< STB_GLOBAL - visible globally
	STB_WEAK   = 2,//!< STB_WEAK - visible globally but with lower
		       //! precedence than STB_GLOBAL types
	STB_LOPROC = 13,
	STB_HIPROC = 15
};

#define ELF32_ST_TYPE(value) ((value) & 0xF)

enum SymbolType
{
	STT_NOTYPE	= 0,//!< STT_NOTYPE - Type not specified
	STT_OBJECT	= 1,//!< STT_OBJECT - Associated with a data object
	STT_FUNC	= 2,//!< STT_FUNC - Associated with a function or other
			    //! executable code
	STT_SECTION	= 3,//!< STT_SECTION - Associated with a section
	STT_FILE	= 4,//!< STT_FILE - Specifies name of a source-file
	STT_LOPROC	= 13,
	STT_HIPROC	= 15
};

#define ELF32_ST_INFO(b, t) (((b) << 4) + ((t)&0xF))

///
/// An entry in the symbol-table of an elf-object file. It holds
/// the information needed to locate and relocate a module's
/// symbolic definition/reference.
///
/// @version 1.0
/// @since Circuit 2.03
/// @author Shukant Pal
///
struct Symbol
{
	ELF32_WORD name;//!< Holds an index into the object file's symbol
			//! string table having its string-name. If it is
			//! zero, then the symbol has no name.
	ELF32_ADDR value;//!< Gives the value of the associated symbol
	ELF32_WORD size;//!< Holds the associated size of the symbol
	unsigned char info;//!< Specifies the symbol's type and binding attr
	unsigned char other;//!< Reserved by specification, currently
	ELF32_HALF sectionIndex;//!< Holds the relevant section-header index
};

///
/// Describes the type of relocation entry and how to alter the
/// relevant instruction and data fields. The descriptions use
/// the following notation -
/// A - This means the addend used to compute the value of the field
/// B - This means the base-address at which module has been loaded in
///     kernel memory.
/// G - This means the offset into the GOT at which the address of the entry's
///     symbol will reside during execution.
/// GOT - This means the address of the GOT
/// L - This means the address of the PLT entry for a symbol
/// P - This means the address of the storage unit being replaced
/// S - This means the value of the symbol whose index resides the entry
///
/// @version 1.0
/// @since Circuit 2.03
/// @author Shukant Pal
///
enum RelocationType
{
	R_386_NONE	= 0,//!< R_386_NONE - none
	R_386_32	= 1,//!< R_386_32 - S + A
	R_386_PC32	= 2,//!< R_386_PC32 - S + A - P
	R_386_GOT32	= 3,//!< R_386_GOT32 - G + A - P
	R_386_PLT32	= 4,//!< R_386_PLT32 - L + A - P
	R_386_COPY	= 5,//!< R_386_COPY - none
	R_386_GLOB_DAT	= 6,//!< R_386_GLOB_DAT - S
	R_386_JMP_SLOT	= 7,//!< R_386_JMP_SLOT - S
	R_386_RELATIVE	= 8,//!< R_386_RELATIVE - B + A
	R_386_GOTOFF	= 9,//!< R_386_GOTOFF - S + A - GOT
	R_386_GOTPC	= 10//!< R_386_GOTPC - GOT + A - P
};

#define ELF32_R_SYM(i) ((i) >> 8)
#define ELF32_R_TYPE(i) ((unsigned char) (i))
#define ELF32_R_INFO(s, t) (((s) << 8) + (unsigned char) (t))

///
/// Relocation entry which has an **implicit addend** located at the
/// field to be relocated.
///
/// @version 1.0
/// @since Circuit 2.03
/// @author Shukant Pal
///
struct RelEntry
{
	ELF32_ADDR offset;//!< Relative-address of relocatable field
	ELF32_WORD info;//!< Gives symbol-table index and type of relocation
};

///
/// Relocation entry which has an **explicit addend** located in the
/// entry itself.
///
/// @version 1.0
/// @since Circuit 2.03
/// @author Shukant Pal
///
struct RelaEntry
{
	ELF32_ADDR offset;//!< Relative-address of relocatable-field
	ELF32_WORD info;//!< Gives symbol-table index & type of relocation
	ELF32_SWORD addend;//!< Constant addend used while relocation
};

///
/// Describes the values by which segments/program-header can be
/// described by the ProgramHeader::entryType field.
///
enum PhdrType
{
	PT_NULL		= 0,//!< PT_NULL - unused entry

	PT_LOAD		= 1,//!< PT_LOAD - specifies loadable segment

	PT_DYNAMIC	= 2,//!< PT_DYNAMIC - specifies dynamic linking info

	PT_INTERP	= 3,//!< PT_INTERP - specifies location and size of
			    //! null terminated path to invoke interpreter. Not
			    //! used by this kernel as ModuleLoader is used.
			    
	PT_NOTE		= 4,//!< PT_NOTE - specifies location and size of any
			    //! auxiliary information. ElfAnalyzer may be used
			    //! to access such kind of notes.

	PT_SHLTB	= 5,//!< PT_SHLTB - reserved as of now

	PT_PHDR		= 6,//!< PT_PHDR - specifies location and size of the
			    //! program-header table in file & memory. It is
			    //! optional but only exists singly.

	PT_LOPROC	= 0x70000000,//!< PT_LOPROC - beginning of CPU-reserved
					//! values

	PT_HIPROC	= 0x7FFFFFFF //!< PT_HIPROC - end of CPU-reserved vals
};

///
/// A kernel module's program-headers describe a segment or other
/// information the kernel-host needs to prepare it for
/// running.
///
/// @version 1.0
/// @since Circuit 2.03
/// @author Shukant Pal
///
struct ProgramHeader
{
	ELF32_WORD entryType;//!< Tells what kind of infomation it gives
	ELF32_OFF fileOffset;//!< Gives the offset in the file at which first
			     //!< byte of segment resides
	ELF32_ADDR virtualAddress;//!< Gives the virtual address at which the
				  //! first byte in the segment resides
	ELF32_ADDR physicalAddress;//!< Not relevant in the kernel-context
	ELF32_WORD fileSize;//!< Size of the segment in the file, may be zero
	ELF32_WORD memorySize;//!< Size of the segment in virtual memory, it
			      //! can be zero
	ELF32_WORD flagSet;//!< Contains the flags relevant to this entry
	ELF32_WORD alignBoundary;//!< Values to which segment are aligned in
				 //! memory and the file
};

///
/// These are the types of **dynamic entries** present in the _DYNAMIC
/// array.
///
/// @version 1.0
/// @since Circuit 2.03
/// @author Shukant Pal
///
enum DynamicTag
{
	DT_NULL 	= 0, //! marks end of dynamic array
	DT_NEEDED 	= 1, //! offset of required library's name
	DT_PLTRELSZ	= 2, //! the total size
	DT_PLTGOT	= 3, //! address of GOT
	DT_HASH		= 4, //! address of the symbol hash table
	DT_STRTAB	= 5, //! address of the string table
	DT_SYMTAB	= 6, //! address of the symbol table
	DT_RELA		= 7, //! address of a relocation table
	DT_RELASZ	= 8, //! total size of the DT_RELA relocation table
	DT_RELAENT	= 9, //! size of the DT_RELA relocation entry
	DT_STRSZ	= 10,//! size of the string table
	DT_SYMENT	= 11,//! size of the symbol table entry
	DT_INIT		= 12,//! address of the initialization function
	DT_FINI		= 13,//! address of the termination function
	DT_SONAME	= 14,//! build name of module
	DT_RPATH	= 15,//! presence in .so file resolution algorithm for
			     //! references within the library
	DT_SYMBOLIC	= 16,//! similar to DT_RELA, has implicit addends
	DT_REL		= 17,//! total size, in bytes, of the DT_REL table
	DT_RELSZ	= 18,//! Element holds the size, in bytes, of the DT_REL relocation entry
	DT_RELENT	= 19,//! Memory specifies the type of relocation entry referred by procedure linkage table
	DT_PLTREL	= 20,//! Member is used for debugging, contents not specified for the ABI, used KAF software only
	DT_DEBUG	= 21,//! absence signifies that no relocation entry causes no change to a non-writable segment
	DT_TEXTREL	= 22,//!< DT_TEXTREL
	DT_JMPREL	= 23,//! pointer to relocation entries for PLT
	DT_LOPROC	= 0x70000000,//! lower-bound for arch-dependent entries
	DT_HIPROC	= 0x7FFFFFFF//! upper-bound for arch-dependent entries
};

///
/// As module participate in dynamic-linking, its PHDRs will have an element
/// of type PT_DYNAMIC which points to the array of these entries containing
/// various dynamic-linking related information.
///
/// @version 1.0
/// @since Circuit 2.3
///
struct DynamicEntry
{
	ELF32_SWORD tag;//!< Controls interpretation of union below
	union
	{
		ELF32_WORD val;//!< Integer value, depending on tag
		ELF32_ADDR ptr;//!< Virtual-address, depending on tag
	};
};

// -- KMod-ELF-Loader CACHE ---

struct SymbolTable
{
	char *nameTable;
	Symbol *entryTable;
	unsigned long entryCount;
};

/*
 * Struct: HashTable
 *
 * Summary:
 * Default-hashTable for storing elf-symbols and getting low lookup-delays
 * while querying symbols.
 *
 * Author: Shukant Pal
 */
struct HashTable
{
	SectionHeader *hashSectionHdr;/* Section-header, optional (@Deprecated) */
	unsigned long bucketEntries;/* No. of bucket entries */
	unsigned long chainEntries;/* No. of chain entries */
	unsigned long *bucketTable;/* Pointer to bucket table */
	unsigned long *chainTable;/* Pointer to chain table */
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

}// namespace Elf
}// namespace Module

#endif/* Module/ELF.h */
