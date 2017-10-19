/*=++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * File: ELF.h
 *
 * Summary:
 * This file contains the ELF-kmodule support types. It is used for loading the
 * kmodules into the kernel-space and linking them dynamically with the microkernel
 * core.
 *
 * Types:
 * ELF32_EHDR, ELF32_SHDR, ELF32_PHDR - As defined in ELF specification
 *
 * @See Portable Formats Specification - ELF
 * Copyright (C) 2017 - Shukant Pal
 *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=*/
#ifndef MODULE_ELF_H
#define MODULE_ELF_H

#include <TYPE.h>

#define EI_NIDENT 16

typedef U32 ELF32_ADDR;
typedef U16 ELF32_HALF;
typedef U32 ELF32_OFF;
typedef S32 ELF32_SWORD;
typedef U32 ELF32_WORD;

typedef enum {
	ELFCLASSNONE=0,// Invalid Class
	ELFCLASS32 = 1,// 32-bit Objects
	ELFCLASS64 = 2 // 64-bit Objects
} ELF32_CLASS;

typedef enum {
	ELFDATANONE = 0,// Invalid Data Encoding
	ELFDATA2LSB = 1,// 2's complement values, with the least significant byte occupying the lowest address
	ELFDATA2MSB = 2 // 2's complement values, with the most significant byte occupying the lowest address
} ELF32_DATA;

typedef enum {
	ET_NONE	= 0,// No file type
	ET_REL	= 1,// Relocatable file
	ET_EXEC	= 2,// Executable file
	ET_DYN	= 3,// Shared-object file
	ET_CORE	= 4,// Core file
	ET_LOPROC	= 0xFF00,// Processor-specific
	ET_HIPROC	= 0xFFFF// Processor-specific
} ELF32_TYPE;

typedef enum {
	EM_NONE	= 0,// No machine
	EM_M32  	= 1,// AT&T WE 32100
	EM_SPARC	= 2,// SPARC
	EM_386 	= 3,// Intel 80386
	EM_68K 	= 4,// Motorola 68000
	EM_88K	= 5,// Motorola 88000
	EM_860	= 7,// Intel 80860
	EM_MIPS	= 8// MIPS RS3000
} ELF32_MACHINE;

#ifdef IA32
	#define EM_RUNNER EM_386
#endif

typedef
enum {
	EV_NONE	= 0, // Invalid Version
	EV_CURRENT = 1 // Current Version
} ELF32_VERSION;

/**
 * Type: ELF_EHDR
 *
 * Summary:
 * The ELF header is represented semantically in this type. It contains all the
 * information to load a KMT_ELF module. The kernel uses this to parse and fill
 * relocation tables in the binary and dynamically link kmodules to core-kernel
 * & other kmodule-dependencies (iff present).
 *
 * @See Executable and Linkable Format
 * @Version 1
 * @Since Circuit 2.03
 * @Author Shukant Pal
 */
typedef struct {
	#define EI_MAG0		0 // File Identification
	#define EI_MAG1		1 // File Identification
	#define EI_MAG2		2 // File Identification
	#define EI_MAG3		3 // File Identification
	#define EI_CLASS		4 // File Class
	#define EI_DATA		5 // Data Encoding
	#define EI_VERSION		6 // File Version
	#define EI_PAD			7 // Start of padding bytes
	#define EI_NIDENT		16// Size of Identifier[]
	
	/* EI_MAG0 to EI_MAG3 */
	#define ELFMAG0		0x7F// EI_MAG0
	#define ELFMAG1		'E' // EI_MAG1
	#define ELFMAG2		'L' // EI_MAG2
	#define ELFMAG3		'F' // EI_MAG3

	UCHAR Identifier[EI_NIDENT];/* Identify the file as an object-file & provide machine-independent data */
	ELF32_HALF Type;/* Identifies the object-file type */
	ELF32_HALF Machine;/* Required architecture for and individual file */
	ELF32_WORD Version;/* Object-file version */
	ELF32_ADDR Entry;/* Virtual address to which the system first transfers control; if not present, 0 */
	ELF32_OFF ProgramHeaderOffset;/* Program Header Table's file offset, in bytes */
	ELF32_OFF SectionHeaderOffset;/* Section Header Table's file offset, in bytes */
	ELF32_WORD MachineFlags;/* Processor-specific flags associated with the file */
	ELF32_HALF HeaderSize;/* ELF header's size in bytes */
	ELF32_HALF ProgramHeaderEntrySize;/* Size of one entry in file's program header table, in bytes */
	ELF32_HALF ProgramHeaderEntryCount;/* No. of entries in the program header table */
	ELF32_HALF SectionHeaderEntrySize;/* Size of section header's size in bytes */
	ELF32_HALF SectionHeaderEntryCount;/* No. of entries in the section header table */
	ELF32_HALF SectionStringIndex;/* Section header table index of entry associated with section name-table */
} ELF32_EHDR;

/* Special Section Indexes */
typedef enum {
	SHN_UNDEF 	= 0, // Undefined, missing, irrelevant or otherwise meaningless section
	SHN_LORESERVE	= 0xFF00, // Lower bound of the range of reserved indexes
	SHN_LOPROC	= 0xFF00, // <Values in this inclusive range are reserved for
	SHN_HIPROC	= 0xFF1F, // processor-specific semantics>
	SHN_ABS		= 0xFFF1, // Absolute values for the corresponding reference
	SHN_COMMON	= 0xFFF2, // Relative to this section are common symbols
	SHN_HIRESERVCE	= 0xFFFF, // Upper bound of the range of reserved indexes
} ELF_SHN;

/* ELF Section Types */
typedef enum {
	SHT_NULL	= 0,
	SHT_PROGBITS= 1,
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
} ELF32_SHT;

/* Section Flags */
typedef enum {
	SHF_WRITE		= 0x1, // Section contains data that should be writable during process execution
	SHF_ALLOC		= 0x2, // Section occupies memory during process exeuction
	SHF_EXECINSTR	= 0x4, // Executuable machine instructions
	SHF_MASKPROC	= 0xF0000000 // Reserved for processor-specific semantics
} ELF32_SHF;

typedef struct {
	ELF32_WORD Name;/* Index into the section header table section (null-terminated) */
	ELF32_WORD Type;/* Section's content and semantics */
	ELF32_WORD Flags;/* Support 1-bit flags that describe miscellaneous attributes */
	ELF32_ADDR Address;/* Address of section's first byte */
	ELF32_OFF Offset;/* Byte offset from the beginning of the file to the first byte in the section */
	ELF32_WORD Size;/* Section's size in bytes */
	ELF32_WORD Link;/* Section header table index link */
	ELF32_WORD Info;/* Holds extra information */
	ELF32_WORD AddressAlign;/* Address alignment constraints */
	ELF32_WORD EntrySize;/* For sections holding entries of fixed-size of this value */
} ELF32_SHDR;

//--------------------------------ELF Symbol-----------------------------------

#define STN_UNDEF			0
#define ELF32_ST_BIND(value) ((value) >> 4)
typedef enum {
	STB_LOCAL	 = 0, // Local symbols, not visible outside the object file
	STB_GLOBAL = 1, // Global symbols, visible outside the object files
	STB_WEAK   = 2, // Represent global symbols of lower precedence
	STB_LOPROC = 13,// Values in range STB_LOPROC
	STB_HIPROC = 15 // to STB_HIPROC are reserved for processor-specific semantics
} ELF32_ST_BIND;

#define ELF32_ST_TYPE(value) ((value) & 0xF)
typedef enum {
	STT_NOTYPE = 0,// Symbol's type is not specified
	STT_OBJECT = 1,// Symbol is associated with a data object
	STT_FUNC   = 2,// Symbol is associated with a function or other executable code
	STT_SECTION= 3,// Symbol is associated with a section, probably for relocation
	STT_FILE   = 4,// Symbol's name gives the name of the source file associated
	STT_LOPROC = 13,
	STT_HIPROC = 15
} ELF32_ST_TYPE;

#define ELF32_ST_INFO(b, t) (((b) << 4) + ((t)&0xF))

typedef struct {
	ELF32_WORD Name;/* Index into object file's string, holding string of symbol name (no name if 0) */
	ELF32_ADDR Value;/* Value of associated symbol */
	ELF32_WORD Size;/* Associated size of the symbol in bytes */
	UCHAR Info;/* Specifies the symbol's type and binding attributes */
	UCHAR Other;/* Currently holds 0 */
	ELF32_HALF SectionIndex;/* Relevant section header table index */
} ELF32_SYM;

//------------------------------ELF Relocation---------------------------------
typedef enum {
	R_386_NONE	= 0,
	R_386_32		= 1,
	R_386_PC32	= 2,
	R_386_GOT32	= 3,
	R_386_PLT32	= 4,
	R_386_COPY	= 5,
	R_386_GLOB_DAT	= 6,
	R_386_JMP_SLOT	= 7,
	R_386_RELATIVE	= 8,
	R_386_GOTOFF	= 9,
	R_386_GOTPC	= 10
} RELOCATION_TYPE;

typedef
struct {
	ELF32_ADDR Offset;
	ELF32_WORD Info;
} ELF32_REL;

typedef struct {
	ELF32_ADDR Offset;/* Location at which to apply relocation action */
	
	#define ELF32_R_SYM(i) ((i) >> 8)
	#define ELF32_R_TYPE(i) ((UCHAR) (i))
	#define ELF32_R_INFO(s, t) (((s) << 8) + (UCHAR) (t))
	ELF32_WORD Info;/* Symbol table index and type of relocation */
	ELF32_SWORD AddEnd;/* Constant addend used to compute the value to be stored into the relocatable field.*/
} ELF32_RELA;

typedef enum {
	PT_NULL		= 0,
	PT_LOAD		= 1,
	PT_DYNAMIC	= 2,
	PT_INTERP		= 3,
	PT_NOTE		= 4,
	PT_SHLTB		= 5,
	PT_PHDR		= 6,
	PT_LOPROC		= 0x70000000,
	PT_HIPROC		= 0x7FFFFFFF
} ELF32_PT;

typedef
struct {
	ELF32_WORD Type;/* Type of segment this array element describes */
	ELF32_OFF Offset;/* Offset from beginning of file at which the first byte of segment resides*/
	ELF32_ADDR VAddr;/* Virtual address of first byte of segment, residing in memory */
	ELF32_ADDR PAddr;/* Physical addressing relevancy only, not used in CircuitKernel-ELF-KModule support! */
	ELF32_WORD FileSize;/* No. of bytes in file image of the segment, may be zero */
	ELF32_WORD MemorySize;/* No. of bytes in memory image of the segment, may be zero */
	ELF32_WORD Flags;/* Has flags relevant to the segment */
	ELF32_WORD Align;/* Gives the value to which segments are aligned in memory & in the file */
} ELF32_PHDR;

typedef enum {
	DT_NULL 		= 	0, // Marks the end of _DYNAMIC array
	DT_NEEDED 	=	1, // Element holds the string table offset of a null-terminated string
	DT_PLTRELSZ	=	2, // Element holds the total size
	DT_PLTGOT		=	3, // Element holds an address associated with the procedure linkage table
	DT_HASH		=	4, // Element holds an address of the symbol hash table
	DT_STRTAB		=	5, // Element holds the address of the string table
	DT_SYMTAB		=	6, // Element holds the address of the symbol table
	DT_RELA		=	7, // Element holds the address of a relocation table
	DT_RELASZ		=	8, // Element holds the total size
	DT_RELAENT	=	9, // Element holds the size
	DT_STRSZ		=	10,// Element holds the size, in bytes, of the DT_RELA relocation entry
	DT_SYMENT		=	11,// Element holds the size, in bytes, of the string table
	DT_INIT		=	12,// Element holds the size, in bytes, of the symbol table entry
	DT_FINI		=	13,// Element holds the address of the initialztion function
	DT_SONAME		=	14,// Element holds the address of the termination function
	DT_RPATH		=	15,// Element holds the string table offset of a null-terminated string
	DT_SYMBOLIC	=	16,// Element's presence in .so file resolution algorithm for references within the library
	DT_REL		=	17,// Element is similar to DT_RELA, except its table has implicit addends, such as ELF32_REL
	DT_RELSZ		=	18,// Element holds total size, in bytes, of the DT_REL relocation table
	DT_RELENT		=	19,// Element holds the size, in bytes, of the DT_REL relocation entry
	DT_PLTREL		=	20,// Member specifies the type of relocation entry referred by procedure linkage table
	DT_DEBUG		=	21,// Member is used for debugging, contents not specified for the ABI, not used in CircuitKernel
	DT_TEXTREL	=	22,// Member's absence signifies that no relocation entry causes no change to a non-writable seg
	DT_JMPREL		=	23,// If present, this entries's' Pointer holds the address of relocation entries associated with PLT
	DT_LOPROC		=	0x70000000,//  For processor-specific...
	DT_HIPROC		=	0x7FFFFFFF// semantics
} DTAG;

typedef struct {
	ELF32_SWORD Tag;
	union {
		ELF32_WORD Value;
		ELF32_ADDR Pointer;
	};
} ELF32_DYN;

// ----------------------------------- ELF Default Types (kernel-specific, not in specs) -----------

#ifdef IA32
	typedef ELF32_CLASS 	ELF_CLASS;
	typedef ELF32_DATA 		ELF_DATA;
	typedef ELF32_TYPE 		ELF_TYPE;
	typedef ELF32_MACHINE 	ELF_MACHINE;
	typedef ELF32_EHDR 		ELF_EHDR;
	
	typedef ELF32_SHT		ELF_SHT;
	typedef ELF32_SHDR 		ELF_SHDR;

	typedef ELF32_PT		ELF_PT;
	typedef ELF32_PHDR 		ELF_PHDR;
	
	typedef ELF32_ST_TYPE		ELF_STT;
	typedef ELF32_SYM 	ELF_SYM;

	typedef ELF32_DYN ELF_DYN;
	typedef ELF32_REL ELF_REL;
	typedef ELF32_RELA ELF_RELA;
#else
	typedef ELF64_CLASS 	ELF_CLASS;
	typedef ELF64_DATA 		ELF_DATA;
	typedef ELF64_TYPE 		ELF_TYPE;
	typedef ELF64_MACHINE 	ELF_MACHINE;
	typedef ELF64_EHDR 		ELF_EHDR;
	
	typedef ELF64_SHT		ELF_SHT;
	typedef ELF64_SHDR 		ELF_SHDR;

	typedef ELF64_PT		ELF_PT;
	typedef ELF64_PHDR 		ELF_PHDR;
	
	typedef ELF64_STT		ELF_STT;
	typedef ELF64_SYM 	ELF_SYM;
#endif

// -- KMod-ELF-Loader CACHE ---

/**
 * Type: KMOD_ECACHE
 *
 * Summary:
 * Module ELF-Cache contains the direct references to important parts of an ELF
 * kmodule. Without this caching mechanism, the kmodule-loader would experience
 * a overhead due to 'repeated' searches for the required ELF sections, symbol 
 * tables, program headers, dynamic tags, etc. whatsoever.
 *
 * @See ModuleRecord::KMOD_RECORD <KTERM_SYNTAX>
 * @Version 1.1
 * @Since Circuit 2.03
 * @Author Shukant Pal
 */
typedef struct {
	CHAR *eSectionNames;
	struct {
		CHAR *eSymbolNames;
		ELF_SYM *eSymbolTable;
		SIZE eSymbolTableLength;
	};
	struct {
		CHAR *eDynamicSymbolNames;
		ELF_SYM *eDynamicSymbolTable;
		SIZE eDynamicSymbolTableLength;
	};

	ELF32_DYN *_DYNAMIC;// Not following naming convention, I know
	SIZE eDynTableLength;

	struct {	
		ELF_RELA *eRelaTable;
		SIZE eRelaCount;
	};
	struct {
		ELF_REL *eRelTable;
		SIZE eRelCount;
	};
	
	VOID *ePLT;
	VOID *eGOT;

	ELF_EHDR *eHeader;
} KMOD_ECACHE;


BOOL MdCheckELFCompat(
	ELF32_EHDR *eHeader
);

#ifdef ARCH_32

struct _KMOD_RECORD;
VOID MdLoadELF(
	ELF32_EHDR *eHeader,
	struct _KMOD_RECORD *kmRecord
);

#else // ARCH_64

VOID MdLoadELF(
	ELF64_EHDR *eHeader
);

#endif

#endif/* Module/ELF.h */
