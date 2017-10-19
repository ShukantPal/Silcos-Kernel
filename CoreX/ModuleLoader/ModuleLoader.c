#define AI_MODLOADER
#define AI_VMM
#include <KERNEL.h>

VOID elf_dbg() { Dbg("ELF PROGRAME CALLS MK");}

OBINFO *tKMOD_RECORD;
LINKED_LIST LoadedKModules;

BOOL MdLoadFile(PADDRESS moduleAddress, ULONG moduleSize, CHAR *cmdLine){
	KMOD_RECORD *kmRecord = KNew(tKMOD_RECORD, KM_SLEEP);

	moduleSize = NextPowerOf2(moduleSize);
	Dbg("__mdlodfil ");
	
	#ifdef ARCH_32
		return_if(moduleSize > MB(2), FALSE);
	#else // ARCH_64
		return_if(moduleSize > MB(4), FALSE);
	#endif

	VOID *moduleLoader = (VOID *) KiPagesAllocate(HighestBitSet(moduleSize), ZONE_KMODULE, FLG_NONE);
	EnsureAllMappings((ULONG) moduleLoader, moduleAddress, moduleSize, NULL, PRESENT | READ_WRITE);
	
	DbgInt((ADDRESS)moduleLoader/KB(1));
	
	if(MdCheckELFCompat(moduleLoader)){
		ELF_EHDR *eHeader = moduleLoader;
		MdLoadELF(eHeader, kmRecord);

		return (TRUE);
	}

	return (FALSE);
}

VOID MdSetupLoader(){
	tKMOD_RECORD = KiCreateType("Module::KMOD_RECORD", sizeof(KMOD_RECORD), sizeof(ULONG), NULL, NULL);
}
