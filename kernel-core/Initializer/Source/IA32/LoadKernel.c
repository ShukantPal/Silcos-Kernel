/**
 * @file LoadKernel.c
 *
 * Logical entry point of the kernel initializer, which parses the
 * multiboot table and loads the kernel environment. If the kernel
 * is found capable of executing, <tt>HostEntry</tt> is executed,
 * otherwise an error message is displayed on the screen, if
 * possible.
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
#define HOST_BUFFER	(1024*36) /* should be >= AUTO_DAT (@see KernHost) */
#define PAGE		(1024*4)
#define HUGE_PAGE	(1024*1024*2)

typedef void (*HostEntry)(ulong tags, ulong evBase,
		ulong evSize, struct KernelSymbol *evSymbols, char **strtabPtrs,
		struct KernelSymbol **evHash);

/**
 * Collects all the arguments required by the main kernel to run, and does
 * a last-time check for undefined symbols. If the kernel is found ready to
 * run, <tt>callMain</tt> continues and jumps to <tt>HostEntry</tt> in the
 * host module.
 *
 * If an undefined symbols is found, a error message is displayed and the
 * kernel is configured to hang indefinitely.
 *
 * @param envBase - the base physical address of the kernel environment
 * @param envSize - the size, in bytes, of the loaded kernel environment,
 * 			excluding any extra data, like the page-frame table.
 */
void CallHost(ulong envBase, ulong envSize)
{
	extern struct KernelModule *linkerCache;/* Relocator.c */
	extern union MultibootSearch firstTag;/* EarlyMultiboot.c */
	extern bool __linkError;/* KernelModule.c */

	struct KernelModule *host = linkerCache + FindHostIndex();
	unsigned (*callMain)(ulong, ulong, ulong,
			struct KernelSymbol *, ulong,
			char **, struct KernelSymbol **,
			struct KernelModule *) =
		(void*)(host->realBase + host->fileHdr->entryAddress);

	/* Comment this out if you don't want me to disturb you
	 * due to undefined symbols (not-allowed-for-Silcos) */
	if(__linkError) {
		WriteLine("Undefined symbols found! Stopping..");
		while(true);
	}

	WriteLine("Calling entry to host...");
	unsigned x  = callMain(firstTag.loc - 8, envBase, envSize,
			evSymbolTable, copiedSymbols,
			strtabByIndex, evHash, linkerCache);

	WriteInteger(x) ;
	while(true);
}

/**
 * Controls the pre-kernel phase of the booting process; initializes
 * text-output, parses the multiboot-table and scans for all loaded
 * kernel-modules. Each module is copied and extracted into
 * permanently allocated physical memory, called the kernel environment
 * which also holds other metadata.
 *
 * Finally, the <tt>KernelHost</tt> module is given control, which
 * in turn follows the kernel boot protocol. If any failure occurs, it
 * is not handled and the kernel hangs displaying an error.
 * memory to hold the initial kernel environment, after which no extra memory
 * is used in the initializer.
 *
 * @param magic - the multiboot magic number passed by the bootloader in
 * 				the EAX register.
 * @param tagTable - physical address of the multiboot table passed in
 * 				the EBX register.
 */
void LoadKernel(uint magic, ulong tagTable)
{
	if(magic != MULTIBOOT2_BOOTLOADER_MAGIC)
		return;

	InitConsole((unsigned char *) 0xb8000);
	InitMultiboot2Access(tagTable);
	WriteLine("Bootstrapping the Silcos kernel environment...");

	ulong moduleCount = 0;
	ulong segmentSpace = 0;
	ulong symbols = 0, stringTableSz = 0;
	ScanAllModules(&moduleCount, &segmentSpace,
			&symbols, &stringTableSz);

	/*
	 * This is the memory required by the kernel environment to load. This
	 * includes module-segments, symbol-tables, symbol name string-tables,
	 * and the buffer for the KernelHost.
	 */
	ulong envBuffer = segmentSpace +
			(symbols * (sizeof(struct KernelSymbol) + // for storing-symbols
					sizeof(struct KernelSymbol*))) + // for hash-table
			128 + /* A buffer */
			(stringTableSz + (moduleCount * sizeof(ulong))) +
			(moduleCount * sizeof(struct KernelModule)) +
			(HOST_BUFFER);
	envBuffer = PAGES_SIZE(envBuffer);
	ulong envBase = FindEnvSpace(envBuffer);

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


