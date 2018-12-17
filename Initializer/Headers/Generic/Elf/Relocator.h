/**
 * @file Relocator.h
 *
 * Provides a minimal dynamic-linker for ELF objects.
 *
 * @version 1.0
 * @since Silcos 3.05
 * @author Shukant Pal
 */
#ifndef INITIALIZER_GENERIC_RELOCATOR_H_
#define INITIALIZER_GENERIC_RELOCATOR_H_

#include <Module/Elf/ELF.h>
#include "KernelModule.h"

extern struct KernelSymbol *evSymbolTable;
extern unsigned long evSymbolCount;
extern unsigned long copiedSymbols;
extern char **strtabByIndex;
extern struct KernelSymbol **evHash;
extern unsigned long evBase;

struct KernelSymbol *SearchGlobalSymbol(char *string);

void InitRelocator(struct KernelModule *envObjectCache,
		struct KernelSymbol *envSymbolTable, unsigned long envSymbolCount,
		unsigned long moduleCount, unsigned long segmentBuffer);
unsigned long CacheModule(void *file);
void RelocateModule(struct KernelModule *mod);
void RelocateAllModules();

#endif/* Generic/Relocator.h */
