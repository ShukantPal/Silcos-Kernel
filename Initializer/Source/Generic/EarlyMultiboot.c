/**
 * @file EarlyMultiboot.c
 *
 * Implements the required functions to access and extract information
 * from the multiboot-information table.
 *
 *  Created on: 10-Jun-2018
 *      Author: sukantdev
 */
#define _CBUILD
#include <ArchGeneric/MemConfig.h>
#include <Generic/Elf/BinaryAnalyzer.h>
#include <Generic/EarlyConsole.h>
#include <Generic/EarlyMultiboot.h>
#include <Generic/Elf/Relocator.h>
#include <Memory/KFrameManager.h>
#include <Utils/CtPrim.h>

union MultibootSearch firstTag;
union MultibootSearch endSearch;

void InitMultiboot2Access(unsigned long accessPointer)
{
	firstTag.loc = accessPointer + 8;
	endSearch.loc = accessPointer + *(unsigned long *) accessPointer;
}

/**
 * Find the first tag with the specified type in the multiboot information
 * table.
 *
 * @param tt - The type of the tag required by the caller.
 * @return - A generic pointer to the first tag of the given type.
 */
union MultibootSearch FindTagByType(unsigned long tt)
{
	union MultibootSearch genPtr = { .loc = firstTag.loc };

	while(genPtr.loc < endSearch.loc) {
		if(genPtr.tag->type == (U32) tt)
			return (genPtr);
		FetchAdjacentTag(&genPtr);
	}

	WriteLine("Warning: Required multiboot-tag not found!");
	genPtr.loc = null;
	return (genPtr);
}

/**
 * Scans all the mmap-entries and calculates the total system memory
 * or physical RAM present.
 *
 * @return - the total physical memory present in the system where
 * 			this kernel is executing.
 */
U64 ScanSystemMemory()
{
	struct MultibootTagMMap *mmap =
			FindTagByType(MULTIBOOT_TAG_TYPE_MMAP).mmap;
	unsigned long mmapBound = (unsigned long) mmap + mmap->size;
	struct MultibootMMapEntry *ent = (struct MultibootMMapEntry *)
				(mmap + 1);

	U64 sysmem = 0;

	while((unsigned long) ent < mmapBound) {
		sysmem += ent->length;
		ent = (struct MultibootMMapEntry *)
				((unsigned long) ent + mmap->entrySize);
	}

	return (sysmem);
}

/**
 * Scans all the modules and returns the total segment space , the no. of
 * symbols found, and the added size of the string tables.
 *
 * @param[out] segmentSpace - The total amount of space to copy the segments
 * 						of all the modules.
 * @param[out] symbols - The total no. of symbols found in kernel modules.
 * @param[out] string - The added size of the string tables to hold the names
 * 					of all known kernel symbols.
 * @since Silcos 3.05
 * @author Shukant Pal
 */
void ScanAllModules(unsigned long *_moduleCount, unsigned long *segmentSpace,
		unsigned long *symbols, unsigned long *strings)
{
	union MultibootSearch genPtr = { .loc = firstTag.loc };
	void *file;

	while(genPtr.loc < endSearch.loc) {
		if(genPtr.tag->type == MULTIBOOT_TAG_TYPE_MODULE) {
			file = (void *)(genPtr.mod->moduleStart);
			if(!IsElfFile(file)) {
				WriteLine("Warning: Non-elf module-files not supported!");
				WriteLine("Not loading unknown file!");
				continue;
			}

			++(*_moduleCount);

			*segmentSpace += ScanSegments((struct ElfHeader *) file);
			*segmentSpace += ARCH_PAGESZ * 4;
			/* Keep a gap of 4 pages between two modules */

			ScanSymbols((struct ElfHeader *) file, symbols, strings);
		}

		FetchAdjacentTag(&genPtr);
	}
}

/**
 * Calls CacheModule() on all the kernel-modules found during setup. This
 * allows them to be linked afterwards.
 *
 * @version 1.0
 * @since Silcos 3.05
 * @author Shukant Pal
 */
void ScanAllModules2()
{
	union MultibootSearch genPtr = { .loc = firstTag.loc };

	while(genPtr.loc < endSearch.loc) {
		if(genPtr.tag->type == MULTIBOOT_TAG_TYPE_MODULE){
			CacheModule((void *)(genPtr.mod->moduleStart));
		}

		FetchAdjacentTag(&genPtr);
	}
}

static inline bool IsValidBounds(unsigned long *addr, unsigned long length,
		unsigned long envSize)
{
	if(length < envSize)
		return (false);

	unsigned int _addr = *addr;
	unsigned int alignAddr = (_addr % ARCH_HPAGESZ) ?
			ALIGNED_HPAGE(_addr) + ARCH_HPAGESZ : _addr;

	if(length < alignAddr - _addr)
		return (false);

	length -= alignAddr - _addr;
	ARCH_PAGE_ALIGN(length);

	if(length < envSize)
		return (false);

	*addr = alignAddr;
	return (true);
}

/**
 * Finds a continuous space in physical memory where the initial
 * kernel environment can be loaded. The base-address returned is
 * usually huge-paged aligned, so that the kernel can be mapped at
 * the immediate higher-half.
 *
 * Available memory near and above PREF_ENVBASE, is checked for
 * availability first. If none is usable, then memory below PREF_ENVBASE
 * is checked (not implemented).
 *
 * (not implemented): If no memory is still available, space is
 * checked again for non-HPAGE alignment.
 *
 * ------ Important Note --------------
 *
 * In addition to the envMem given, this function also allocates memory
 * for the page-frame allocator's (in KernelHost) entries. This are
 * placed just after the kernel-environment in physical memory. They are
 * implicitly used, in the page-frame allocator initialization.
 *
 * @param envMem - The size of the kernel environment, e.g. memory
 * 				required by the module segments.
 * @return - The base-address where the kernel-environment will be
 * 			loaded.
 * @version 1.0
 * @since Silcos 3.05
 * @author Shukant Pal
 */
unsigned long FindEnvSpace(unsigned long envMem)
{
	envMem = HUGE_PAGES_SIZE(envMem);
	envMem += (ScanSystemMemory() >> 12) * sizeof_mmframe;// for page-frames

	struct MultibootTagMMap *physmap =
			FindTagByType(MULTIBOOT_TAG_TYPE_MMAP).mmap;

	if(physmap == null)
		WriteLine("Warning: Memory-map located at address 0x00000000.");

	unsigned long endMapSearch = (unsigned long) physmap + physmap->size;
	unsigned long entSize = physmap->entrySize;

	struct MultibootMMapEntry *mpref = null;
	union MultibootMMapSearch cur = {
			.entPtr = (unsigned long)(physmap + 1)
	};

	while(cur.entPtr < endMapSearch) {
		if(cur.ent->address + cur.ent->length >= PREF_ENVBASE) {
			mpref = cur.ent;
			break;
		}
		cur.entPtr += entSize;
	}

	if(mpref->type == MULTIBOOT_MEMORY_AVAILABLE) {
		unsigned long mprefAvail = mpref->length -
				(PREF_ENVBASE - mpref->address);
		if(mprefAvail >= envMem)
			return (PREF_ENVBASE);
	}

	cur.entPtr += entSize;
	unsigned long envBase;
	while(cur.entPtr < endMapSearch) {
		if(cur.ent->type == MULTIBOOT_MEMORY_AVAILABLE) {
			envBase = cur.ent->address;
			if(IsValidBounds(&envBase, cur.ent->length, envMem))
				return (envBase);
		}
		cur.entPtr += entSize;
	}

	WriteLine("Fatal Error: System does not have enough memory to hold the kernel!");
	Write("Kernel Load Requirement: "); WriteInteger(envMem >> 10); WriteLine("KB");
	Write("Found System Memory: "); WriteInteger(ScanSystemMemory() >> 10); WriteLine("KB");
	while(true) { asm volatile("hlt"); }
	return (0);

	/* TODO: Add support for loading kernels under PREF_ENVBASE */
}

unsigned FindHostIndex()
{
	union MultibootSearch genPtr = { .loc = firstTag.loc };
	unsigned index = 0;

	while(genPtr.loc < endSearch.loc) {
		if(genPtr.tag->type == MULTIBOOT_TAG_TYPE_MODULE) {
			if(strcmpn(genPtr.mod->CMD_Line, "KernelHost", 10))
				return (index);
			WriteLine("found mod ");
			++(index);
		}

		FetchAdjacentTag(&genPtr);
	}

	WriteLine("Fatal Error: KernelHost not found! Nothing to execute!");
	WriteLine("Message: Reboot your system, no inputs found! Hanging... now");
	while(TRUE){ asm volatile("hlt"); }
	return (unsigned)(-1);
}
