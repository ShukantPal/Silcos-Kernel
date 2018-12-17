/**
 * @file Relocator.c
 *
 * Finds R_386_RELATIVE relocations in the KernelHost and resolves them so the
 * KernelHost can execute.
 *
 *  Created on: 09-Jun-2018
 *      Author: sukantdev
 */
#define _CBUILD
#include <ArchGeneric/MemConfig.h>
#include <Generic/EarlyConsole.h>
#include <Generic/Elf/Relocator.h>
#include <Module/Elf/ELF.h>
#include <Utils/CtPrim.h>

struct KernelModule *linkerCache;
struct KernelModule *bufferPointer;
unsigned long moduleCount;

struct KernelSymbol *evSymbolTable;
struct KernelSymbol *evSymbolBuffer;
unsigned long evSymbolCount;
unsigned long copiedSymbols = 0;

char **strtabByIndex;
char *strtabBuffer;

struct KernelSymbol **evHash;
unsigned long segmentCopyBuffer;

unsigned long evBase;

/**
 * Fills the corresponding bucket in the hash-table for the given symbol,
 * without looking for a already existent symbol. This is because the
 * symbols will be fully loaded before linkage. Therefore, if any two
 * symbols with the same name exist, only the first one in the list will
 * be used, because the search for a symbol ends on finding the first
 * match.
 *
 * @param sym - the KernelSymbol object to be added to the symbol hash
 */
static void FillBucket(struct KernelSymbol *sym)
{
	unsigned long bucketIndex = HashString(sym->name) % evSymbolCount;

	struct KernelSymbol *next = evHash[bucketIndex];
	sym->next = next;
	if(next)
		next->last = sym;
	evHash[bucketIndex] = sym;
}

static void CopySymbol(struct Symbol *elfSym, struct KernelModule *mod)
{
	if(elfSym->value == 0)
		return;

	++(copiedSymbols);

	struct KernelSymbol *sym = evSymbolBuffer++;

	sym->virtualAddress = elfSym->value + mod->virtualBase;
	sym->ownerIndex = mod->orderIndex;
	sym->name = strtabByIndex[mod->orderIndex] + elfSym->name;

	FillBucket(sym);
}

struct KernelSymbol *SearchGlobalSymbol(char *name)
{
	unsigned long bucket = HashString(name) % evSymbolCount;
	struct KernelSymbol *sym = evHash[bucket];

	while(sym) {
		if(strcmp(name, sym->name))
			return (sym);
		sym = sym->next;
	}

	return (null);
}

/**
 * Initializes the kernel-module loading process by pre-fetching all the
 * required fields and tables. This includes the program-header,
 * dynamic-section, relocation-tables, symbol-table, symbol-hashes, etc.
 *
 * Until this function is called on a KernelModule object, it cannot be
 * used to load and link a ELF file.
 *
 * @param ldrObj - The module which is going to be loaded and linked
 * 				dynamically, and for which initialization is required.
 */
void InitLoader(struct KernelModule *ldrObj)
{
	struct ElfHeader *ldrMap = ldrObj->fileHdr;

	ldrObj->segments = (struct ProgramHeader *)
			(ldrObj->fileBase + ldrMap->programHeaderOffset);
	ldrObj->segEntryCount = ldrMap->programHeaderEntryCount;

	ldrObj->segEntrySize = ldrMap->programHeaderEntrySize;
}

/**
 * Copies the segments of the module into segmentCopyPointer. After copying
 * finishes, the copy-pointer is updated by adding the total memory used by
 * the the segments and a page-size buffer.
 *
 * @param ldrObj
 */
void CopySegments(struct KernelModule *ldrObj)
{
	ldrObj->realBase = segmentCopyBuffer;
	ldrObj->virtualBase = segmentCopyBuffer + KERNEL_OFFSET - evBase;
	struct ProgramHeader *segEnt = ldrObj->segments;
	unsigned long farthestAddress = 0, segLimit;

	for(unsigned index = 0; index < ldrObj->segEntryCount; index++, segEnt++) {
		if(segEnt->entryType != PT_LOAD)
			continue;

		segLimit = segEnt->virtualAddress + segEnt->memorySize;

		if(segLimit > farthestAddress)
			farthestAddress = segLimit;

		memcpy((const void*)(ldrObj->fileBase + segEnt->fileOffset),
				(void*)(ldrObj->realBase + segEnt->virtualAddress),
				segEnt->fileSize);

		if(segEnt->memorySize > segEnt->fileSize)
			memsetf((void*)(ldrObj->realBase +
					segEnt->virtualAddress + segEnt->fileSize),
					0,
					segEnt->memorySize - segEnt->fileSize);
	}

	farthestAddress = PAGES_SIZE(farthestAddress);
	farthestAddress += ARCH_PAGESZ;

	segmentCopyBuffer += farthestAddress;
}

/**
 * Copies the string table of the kernel-module into the global "environment"
 * string table (@code strtabBufferPointer). It increments the current pointer
 * of the global string table by the size of the local string table being
 * copied.
 *
 * It also updates the stringTables pointer table to point to the string table
 * of this module.
 */
void CopyStringTable(struct KernelModule *ldrObj)
{
	struct DynamicEntry *strTable = SearchLinkerTag(ldrObj, DT_STRTAB);
	struct DynamicEntry *strSz = SearchLinkerTag(ldrObj, DT_STRSZ);

	if(strTable == null) {
		WriteLine("Warning: No string table found!");
		return;
	}

	char *fileStrtab = (char *)(ldrObj->realBase + strTable->ptr);
	unsigned long fileStrtabSz = strSz->val;

	strtabByIndex[ldrObj->orderIndex] = strtabBuffer;
	memcpy(fileStrtab, strtabBuffer, fileStrtabSz);
	strtabBuffer += fileStrtabSz;
}

/**
 * Uses the .dynamic segment to locate and cache the relocation tables present
 * in the kernel module. It finds the DT_REL, DT_RELA, and DT_JMPREL
 * entries and then stores the data into the kernel-module object.
 *
 * @param ldrObj
 * @return
 */
void FindRelocationTables(struct KernelModule *ldrObj)
{
	struct ProgramHeader *dynSeg = SearchSegment(ldrObj, PT_DYNAMIC);
	ldrObj->linkerTable = (struct DynamicEntry*)
			(ldrObj->realBase + dynSeg->virtualAddress);

	struct DynamicEntry *relocTable, *relocEntrySize, *relocTotalSize;

	relocTable = SearchLinkerTag(ldrObj, DT_REL);
	relocEntrySize = SearchLinkerTag(ldrObj, DT_RELENT);
	relocTotalSize = SearchLinkerTag(ldrObj, DT_RELSZ);

	if(relocTable != null) {
		ldrObj->rel.tableLocation = ldrObj->realBase + relocTable->ptr;
		ldrObj->rel.entrySize = relocEntrySize->val;
		ldrObj->rel.entryCount = relocTotalSize->val / relocEntrySize->val;
		ldrObj->rel.relocType = DT_REL;
	} else {
		ldrObj->rel.entryCount = 0;
		ldrObj->rel.entrySize = sizeof(struct RelEntry);
	}

	relocTable = SearchLinkerTag(ldrObj, DT_RELA);
	relocEntrySize = SearchLinkerTag(ldrObj, DT_RELAENT);
	relocTotalSize = SearchLinkerTag(ldrObj, DT_RELASZ);

	if(relocTable != null) {
		ldrObj->rela.tableLocation = ldrObj->realBase + relocTable->ptr;
		ldrObj->rela.entrySize = relocEntrySize->val;
		ldrObj->rela.entryCount = relocTotalSize->val / relocEntrySize->val;
		ldrObj->rela.relocType = DT_RELA;
	} else {
		ldrObj->rela.entryCount = 0;
		ldrObj->rela.entrySize = sizeof(struct RelaEntry);
	}

	relocTable = SearchLinkerTag(ldrObj, DT_JMPREL);
	relocTotalSize = SearchLinkerTag(ldrObj, DT_PLTRELSZ);

	if(relocTable != null) {
		ldrObj->pltrel.tableLocation = ldrObj->realBase + relocTable->ptr;
		ldrObj->pltrel.relocType = SearchLinkerTag(ldrObj, DT_PLTREL)->val;

		if(ldrObj->pltrel.relocType == DT_REL) {
			ldrObj->pltrel.entrySize = ldrObj->rel.entrySize;
		} else {
			ldrObj->pltrel.entrySize = ldrObj->rela.entrySize;
		}

		ldrObj->pltrel.entryCount =
				relocTotalSize->val / ldrObj->pltrel.entrySize;
	}
}

void CopySymbols(struct KernelModule *ldrObj)
{
	struct DynamicEntry *symTab = SearchLinkerTag(ldrObj, DT_SYMTAB);
	struct DynamicEntry *hashTab = SearchLinkerTag(ldrObj, DT_HASH);

	unsigned long symCount = ((unsigned long*)
			(ldrObj->realBase + hashTab->ptr))[1];

	struct Symbol *sym = (struct Symbol *)(ldrObj->realBase + symTab->ptr);

	ldrObj->dynSyms.entryTable = sym;
	ldrObj->dynSyms.entryCount = symCount;
	ldrObj->dynSyms.nameTable = strtabByIndex[ldrObj->orderIndex];

	for(unsigned index = 0; index < symCount; index++, sym++)
		CopySymbol(sym, ldrObj);
}

/**
 * Initializes the relocator to deal with the fixed memory buffer allocated
 * for the environment. It locates the linker-cache, global environment symbol
 * table, hash-table, and a global string table in the given buffer.
 *
 * @param envObjectCache - the buffer allocated for storing KernelModule
 * 						objects.
 * @param envSymbolTable - the buffer allocated for the symbol table,
 * 						hash-table, and the string-table. They are located
 * 						in this order.
 * @param envSymbolCount - the total number of symbol found in all the
 * 						modules.
 * @param _moduleCount - the total number of modules found!
 * @version 1.0
 * @since Silcos 3.05
 * @author Shukant Pal
 */
void InitRelocator(struct KernelModule *envObjectCache,
		struct KernelSymbol *envSymbolTable, unsigned long envSymbolCount,
		unsigned long _moduleCount, unsigned long segmentCopy)
{
	moduleCount = _moduleCount;

	linkerCache = envObjectCache;
	bufferPointer = linkerCache;

	evSymbolTable = envSymbolTable;
	evSymbolBuffer = evSymbolTable;
	evSymbolCount = envSymbolCount;

	evHash = (struct KernelSymbol **)(envSymbolTable + envSymbolCount);

	strtabByIndex = (char **)((unsigned long) envSymbolTable +
			envSymbolCount *
			(sizeof(struct KernelSymbol) + sizeof(struct KernelSymbol*))
			+ 128 /*a buffer*/);

	strtabBuffer = (char *)(strtabByIndex + moduleCount);
	segmentCopyBuffer = segmentCopy;

	evBase = segmentCopy;
}

unsigned long lastOrder = 0;

/**
 * Scans the module file given, and adds it in the linker-cache, so that
 * it can be dynamically-linked afterwards. On doing so, the a KernelModule
 * object is allocated and relevant parts of the ELF object are cached.
 *
 * If the ELF-object given is faulty, then no resources will be
 * allocated, but no error code will be returned.
 *
 * @param file - The physical address of where the boot-loader copied the
 * 				module from the disk into memory.
 * @return - The amount of memory required by the module to be loaded.
 * @version 1.0
 * @since Silcos 3.05
 * @author Shukant Pal
 */
unsigned long CacheModule(void *file)
{
	struct KernelModule *loaderObject = bufferPointer++;

	loaderObject->fileHdr = (struct ElfHeader *) file;
	loaderObject->orderIndex = lastOrder++;

	InitLoader(loaderObject);
	CopySegments(loaderObject);
	FindRelocationTables(loaderObject);
	CopyStringTable(loaderObject);
	CopySymbols(loaderObject);

	return (0);
}

void RelocateModule(struct KernelModule *ldrObj)
{
	ResolveAllRel(ldrObj->rel.relEntries, ldrObj, ldrObj->rel.entryCount);
	ResolveAllRela(ldrObj->rela.relaEntries, ldrObj, ldrObj->rela.entryCount);

	if(ldrObj->pltrel.relocType == DT_REL)
		ResolveAllRel(ldrObj->pltrel.relEntries, ldrObj, ldrObj->pltrel.entryCount);
	else
		ResolveAllRela(ldrObj->pltrel.relaEntries, ldrObj, ldrObj->pltrel.entryCount);
}

void RelocateAllModules(struct KernelModule *ldrObj)
{
	struct KernelModule *km = linkerCache;
	for(unsigned idx = 0; idx < moduleCount; idx++, km++)
		RelocateModule(km);
}

