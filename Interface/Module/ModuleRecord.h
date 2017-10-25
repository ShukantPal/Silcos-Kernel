#ifndef __MODULE_RECORD_H__
#define __MODULE_RECORD_H__

#include <Memory/KObjectManager.h>
#include "ELF.h"
#include <Util/LinkedList.h>

// remember, KAF applications are not modules. They are loaded by KAF clients
typedef
enum ModuleType {
	KMT_KDF_DRIVER	= 1,// Kernel-mode Driver
	KMT_EXC_MODULE	= 2,// Executive Module
	KMT_KMCF_EXTEN	= 3,// KMCF Extension (Kmodule Communicator Framework)
	KMT_UNKNOWN	= 4// Unknown (restricted form)
} KM_TYPE;

typedef
struct ModuleRecord {
	union {
		struct ModuleRecord *NextModule;
		struct ModuleRecord *PreviousModule;
		LINODE LiLinker;
	};
	CHAR Name[16];
	ULONG Version;
	KM_TYPE Type;
	KMOD_ECACHE ECache;
	ADDRESS BaseAddr;/* Points to the memory where the module is loaded at. */
} KMOD_RECORD;

/**
 * Function: MdCreateModule
 *
 * Summary:
 * This function gives a module record which is used by all module-operation functions
 * for managing the module state & identity.
 *
 * @Version 1
 * @Since Circuit 2.03
 */
struct ModuleRecord *
MdCreateModule(
		CHAR *moduleName,
		ULONG moduleVersion,
		ULONG moduleType
);

/**
 * Function: MdLoadCoreModule
 *
 * Summary:
 * This function loads the 'core' module. This refers to the kernel-executable's module
 * record, which is required for keeping its symbol table in the kernel module list. Thus,
 * a separate record is kept for the microkernel and added to the ModuleList.
 *
 * @Version 1
 * @Since Circuit 2.03
 * @Author Shukant Pal
 */
static inline
struct ModuleRecord *
MdLoadCoreModule(VOID){
	return MdCreateModule("CORE", 200300, KMT_EXC_MODULE);
}


extern OBINFO *tKMOD_RECORD;
extern LINKED_LIST LoadedModules;

#endif
