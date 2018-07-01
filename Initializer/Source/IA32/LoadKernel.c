/**
 * @file LoadKernel.c
 *
 * This file is the logical entry point of the in-kernel initializer and the
 * startup of the kernel environment. It is responsible for loading multiboot
 * information and extracting available memory for the KernelHost. It also
 * holds the file-to-segment copying logic (@see Module/Elf/ELF.h)
 *
 * @version 0.01
 * @since Silcos 3.05
 * @author Shukant Pal
 */
#define _CBUILD
#include <ArchGeneric/MemConfig.h>
#include <Generic/EarlyConsole.h>
#include <Generic/EarlyMultiboot.h>
#include <Generic/Elf/Relocator.h>
#include <Module/Elf/ELF.h>
#include <Multiboot2.h>
#include <Utils/CtPrim.h>

#define MARK_16M	(1024*1024*16)
#define HOST_BUFFER	(1024*32)
#define PAGE		(1024*4)
#define HUGE_PAGE	(1024*1024*2)

typedef void (*HostEntry)(unsigned long tags, unsigned long evBase,
		unsigned long evSize, struct KernelSymbol *evSymbols, char **strtabPtrs,
		struct KernelSymbol **evHash);

void CallHost(unsigned long envBase, unsigned long envSize)
{
	extern struct KernelModule *linkerCache;/* Relocator.c */
	extern union MultibootSearch firstTag;/* EarlyMultiboot.c */

	struct KernelModule *host = linkerCache + FindHostIndex();
	unsigned (*callMain)(unsigned long, unsigned long, unsigned long,
			struct KernelSymbol *, unsigned long,
			char **, struct KernelSymbol **,
			struct KernelModule *) =
		(void*)(host->realBase + host->fileHdr->entryAddress);

	WriteLine("Calling entry to host...");
	unsigned x  = callMain(firstTag.loc - 8, envBase, envSize,
			evSymbolTable, copiedSymbols,
			strtabByIndex, evHash, linkerCache);

	WriteInteger(x) ;
	while(true);
}

/**
 * The "main" function of the initializer, which performs various operations in
 * a fixed order, and ultimately loads, links and then executes the kernel.
 *
 * During this process, all kernel modules are pre-scanned to check for memory
 * requirements, which includes symbol-tables, string-tables, segment-sizes,
 * other configuration memory. A fixed memory buffer is located in physical
 * memory to hold the initial kernel environment, after which no extra memory
 * is used in the initializer.
 *
 * All kernel modules are then cached - symbols and their names are stored in
 * a central data structure. On doing so, the modules are then relocated to the
 * higher-half address space.
 *
 * Although the symbols of the KernelHost will have values above
 * KERNEL_OFFSET (in virtual memory), it will be first called in physical
 * memory.
 *
 * @param magic
 * @param tagTable
 */
void LoadKernel(unsigned int magic, unsigned long tagTable)
{
	if(magic != MULTIBOOT2_BOOTLOADER_MAGIC)
		return;

	InitConsole((unsigned char *) 0xb8000);
	InitMultiboot2Access(tagTable);
	WriteLine("Bootstrapping the Silcos kernel environment...");

	unsigned long moduleCount = 0;
	unsigned long segmentSpace = 0;
	unsigned long symbols = 0, stringTableSz = 0;
	ScanAllModules(&moduleCount, &segmentSpace,
			&symbols, &stringTableSz);

	/*
	 * This is the memory required by the kernel environment to load. This
	 * includes module-segments, symbol-tables, symbol name string-tables,
	 * and the buffer for the KernelHost.
	 */
	unsigned long envBuffer = segmentSpace +
			(symbols * (sizeof(struct KernelSymbol) + // for storing-symbols
					sizeof(struct KernelSymbol*))) + // for hash-table
			128 + /* A buffer */
			(stringTableSz + (moduleCount * sizeof(unsigned long))) +
			(moduleCount * sizeof(struct KernelModule)) +
			(HOST_BUFFER);
	envBuffer = PAGES_SIZE(envBuffer);
	unsigned long envBase = FindEnvSpace(envBuffer);

	memsetf((void*) envBase + segmentSpace, 0, envBuffer - segmentSpace);

	InitRelocator(
			(struct KernelModule *)(envBase + envBuffer
					- HOST_BUFFER - moduleCount * sizeof(struct KernelModule)),
			(struct KernelSymbol *)(envBase + segmentSpace),
			symbols, moduleCount, envBase
			);

	ScanAllModules2();
	WriteLine("Scanned all modules - preparing for relocation...");
	RelocateAllModules();
	WriteLine("All modules relocated - preparing for execution...");

	CallHost(envBase, envBuffer);
}


