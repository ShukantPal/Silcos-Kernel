/**
 * @file KernelModule.c
 *
 * Implements utility function to access and cache information present in
 * kernel modules relevant to relocation. It deals with KernelModule objects
 * and their properties.
 *
 * @version 1.0
 * @since Silcos 3.05
 * @author Shukant Pal
 */
#define _CBUILD
#include <ArchGeneric/Config.h>
#include <Generic/EarlyConsole.h>
#include <Generic/Elf/KernelModule.h>
#include <Generic/Elf/Relocator.h>
#include <Utils/CtPrim.h>

/**
 * Linearly searches for the program-header entry in the given module
 * for the specified type. It returns a pointer to the entry in the file
 * itself.
 *
 * @param[in] ldrObj - The module in which the header is to be searched
 * @param typeRequired - The type of the program header required.
 * @return
 */
struct ProgramHeader *SearchSegment(struct KernelModule *ldrObj,
		enum PhdrType typeRequired)
{
	unsigned long phdrIndex = 0;
	struct ProgramHeader *phdr = ldrObj->segments;

	while(phdrIndex < ldrObj->segEntryCount) {
		if(phdr->entryType == typeRequired)
			return (phdr);

		++(phdrIndex);
		++(phdr);
	}

	return (null);
}

/**
 * Linearly searches for an entry in the .dynamic segment of the module with
 * the specified tag. It returns a pointer to the entry in the file itself.
 *
 * @param ldrObj - The module in which the entry is to be found
 * @param tagRequired - The type of entry required.
 * @return - A pointer to the dynamic entry in the module's file.
 */
struct DynamicEntry *SearchLinkerTag(struct KernelModule *ldrObj,
		enum DynamicTag tagRequired)
{
	struct DynamicEntry *dyn = ldrObj->linkerTable;

	while(dyn->tag != 0) {
		if(dyn->tag == tagRequired)
			return (dyn);

		++(dyn);
	}

	return (null);
}

/**
 * Resolves a "rel" type of relocation-entry in the kernel-module given. It
 * relies on the relocator to find the symbols required.
 *
 * @param rel
 * @param ldrObj
 */
void ResolveRel(struct RelEntry *rel, struct KernelModule *ldrObj)
{
	struct Symbol *refSym =
			ldrObj->dynSyms.entryTable + ELF32_R_SYM(rel->info);
	unsigned long *patch = (unsigned long *)(ldrObj->realBase + rel->offset);

	if(refSym->name == null
			&& ELF32_R_TYPE(rel->info) == R_386_RELATIVE) {
		*patch += ldrObj->virtualBase;
		return;
	}

	struct KernelSymbol *defSym =
			SearchGlobalSymbol(ldrObj->dynSyms.nameTable + refSym->name);

	if(defSym == null) {
		Write("Undefined Symbol: ");
		WriteLine(ldrObj->dynSyms.nameTable + refSym->name);
		return;
	}

//#if ARCH == IA32 (doggy Eclipse IDE can digest this)
	switch(ELF32_R_TYPE(rel->info)) {
	case R_386_32:
		*patch += defSym->virtualAddress;
		break;
	case R_386_PC32:
		*patch += defSym->virtualAddress;

		/*
		 * Really careful, we subtract the location of the
		 * patcing to ELF, but actually must subtract the
		 * virtual address of the patch here.
		 */
		*patch -= ldrObj->virtualBase + rel->offset;
		break;
	case R_386_GLOB_DAT:
	case R_386_JMP_SLOT:
		*patch = defSym->virtualAddress;
		break;
	case R_386_NONE:
	case R_386_COPY:
		return;
	default:
		WriteLine("Warning: Unknown relocation type (rel) found!");
		while(true);
		break;
	}
//#endif
}

/**
 * Resolves a 'rela' relocation entry in a kernel-module.
 *
 * @param rela
 * @param ldrObj
 */
void ResolveRela(struct RelaEntry *rela, struct KernelModule *ldrObj)
{
	struct Symbol *refSym =
			ldrObj->dynSyms.entryTable + ELF32_R_SYM(rela->info);
	struct KernelSymbol *defSym =
			SearchGlobalSymbol(ldrObj->dynSyms.nameTable + refSym->name);
	unsigned long *patch = (unsigned long *)(ldrObj->realBase + rela->offset);

// #ifdef ARCH == IA32
	switch(ELF32_R_TYPE(rela->info)) {
	case R_386_32:
		*patch = defSym->virtualAddress + rela->addend;
		break;
	case R_386_PC32:
		*patch = defSym->virtualAddress + rela->addend -
				(unsigned long) patch;
		break;
	case R_386_RELATIVE:
		*patch = ldrObj->virtualBase + rela->addend;
		break;
	case R_386_NONE:
	case R_386_COPY:
		return;
	default:
		WriteLine("Warning: Unknown relocation type (rela) found!");
		break;
	}
// #endif
}

void ResolveAllRel(struct RelEntry *rel, struct KernelModule *ldrObj,
		unsigned long count)
{
	for(unsigned int index = 0; index < count; index++, rel++)
		ResolveRel(rel, ldrObj);
}

void ResolveAllRela(struct RelaEntry *rela, struct KernelModule *ldrObj,
		unsigned long count)
{
	for(unsigned int index = 0; index < count; index++, rela++)
		ResolveRela(rela, ldrObj);
}
