#define NS_PMFLGS // Paging Flags

#include <Memory/Pager.h>
#include <Memory/KMemoryManager.h>
#include <Memory/KObjectManager.h>
#include <Module/ModuleLoader.h>
#include <KERNEL.h>

VOID elf_dbg() { Dbg("ELF PROGRAME CALLS MK");}

OBINFO *tKMOD_RECORD;
LINKED_LIST LoadedModules;

/**
 * Function: MdLoadFile
 *
 * Summary:
 * This function is used to load a module which is present in physical
 * memory. It will map the module to virtual memory and invoke the ABI
 * handler for its type. Beware, that this function requires a valid
 * kernel module record. Use, MdLoadRawBinary() to automatically create
 * a kernel module record based on file-contents (not recommended).
 *
 * @Version 1
 * @Circuit 2.03
 */
BOOL MdLoadFile(PADDRESS moduleAddress, ULONG moduleSize, CHAR *cmdLine, KMOD_RECORD *kmRecord){
	moduleSize = NextPowerOf2(moduleSize);
	DbgLine("Module::MdLoadFile { ");
	Dbg("Module Name: "); DbgLine(kmRecord->Name);
	
	#ifdef ARCH_32
		return_if(moduleSize > MB(2), FALSE);
	#else // ARCH_64
		return_if(moduleSize > MB(4), FALSE);
	#endif

	VOID *moduleLoader = (VOID *) KiPagesAllocate(HighestBitSet(moduleSize >> KPGOFFSET), ZONE_KMODULE, FLG_NONE);
	EnsureAllMappings((ULONG) moduleLoader, moduleAddress, moduleSize, NULL, PRESENT | READ_WRITE);

	if(MdCheckELFCompat(moduleLoader)){
		DbgLine("@TransferControl - Module::ELF }");
		ELF_EHDR *eHeader = moduleLoader;
		MdLoadELF(eHeader, kmRecord);

		return (TRUE);
	}

	return (FALSE);
}

VOID MdSetupLoader(){
	tKMOD_RECORD = KiCreateType("Module::KMOD_RECORD", sizeof(KMOD_RECORD), sizeof(ULONG), NULL, NULL);
	ESetupLoader();
}

KMOD_RECORD *MdCreateModule(CHAR *moduleName, ULONG moduleVersion, ULONG moduleType){
	KMOD_RECORD *mdRecord = KNew(tKMOD_RECORD, KM_SLEEP);
	memcpy(moduleName, &mdRecord->Name, 16);
	mdRecord->Version = moduleVersion;
	mdRecord->Type = moduleType;

	AddElement(&mdRecord->LiLinker, &LoadedModules);
	return (mdRecord);
}
