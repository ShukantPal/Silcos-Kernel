/**
 * File: Module/ModuleRecord.h
 *
 * Summary:
 * ModuleRecord provides a management interface to manage module's and their related data
 * base.
 *
 * Author: Shukant Pal
 */
#ifndef __MODULE_RECORD_H__
#define __MODULE_RECORD_H__

#include <Memory/KObjectManager.h>
#include "Elf/ELF.h"
#include "Elf/ElfManager.hpp"
#include <Util/LinkedList.h>

using namespace Module;
using namespace Module::Elf;

// remember, KAF applications are not modules. They are loaded by KAF clients
namespace Module
{

typedef
enum ModuleType {
	KMT_KDF_DRIVER	= 1,// Kernel-mode Driver
	KMT_EXC_MODULE	= 2,// Executive Module
	KMT_KMCF_EXTEN	= 3,// KMCF Extension (Kmodule Communicator Framework)
	KMT_UNKNOWN	= 4// Unknown (restricted form)
} KM_TYPE;

enum ABI
{
	INVALID = 0,
	ELF = 1
};

struct DynamicLink
{
	LinkedList *moduleDependencies;
	SymbolTable dynamicSymbols;
	HashTable symbolHash;
};

/**
 * Struct: ModuleRecord
 *
 * Summary:
 * This struct is used for modeling a runtime-record of a kernel module. It
 * contains ABI information and dynamic-linking information related to the
 * module.
 *
 * It is used by the RecordManager for cross-ABI searches for symbols exported
 * by software-components. Client must initialize it & register it into the
 * RecordManager.
 *
 * Origin:
 * This struct is used for maintaining module records with the RecordManager.
 *
 * Version: 1.0
 * Since: Circuit 2.03
 * Author: Shukant Pal
 */
typedef
struct ModuleRecord
{
	union
	{
		ModuleRecord *NextModule;/* Next-module in list */
		ModuleRecord *PreviousModule;/* Previous-module in list */
		LinkedListNode LiLinker;/* Used for participating the RecordManager list */
	};
	char buildName[16];/* Name given to the module, at build time (16-bytes max., may not null-terminate) */
	unsigned long buildVersion;/* Version of the module build, not service */
	KM_TYPE serviceType;/* Type of service provided by the module */
	DynamicLink *linkerInfo;/* Dynamic link-information */
	void (*init)();/* Initialization function */
	ADDRESS entryAddr;/* Virtual address of entry point */
	void (*fini)();/* Finialization function */
	ADDRESS BaseAddr;/* Address where the module was loaded in kernel memory */
} KMOD_RECORD;

/**
 * Class: RecordManager
 *
 * Summary:
 * This class provides a ABI-independent organization of modules & their
 * associated symbol lookup tables. The RecordManager helps the dynamic
 * linkers to query symbols & module-information globally.
 *
 * It also helps the linker to process multiple inter-dependent modules,
 * each of which the dynamic-link info is exported and inserted into the
 * respective records. Then the relocations are done on each of them.
 *
 * Functions:
 * registerRecord - This function inserts a module-record into global namespace
 * createRecord - Allocates a module-record with a valid name, build, and type
 * querySymbol - Used for querying a symbol in the global namespace by name
 *
 * Version: 1.1
 * Since: Circuit 2.03++
 * Author: Shukant Pal
 */
class RecordManager
{
public:
	static inline void registerRecord(ModuleRecord *modRecord)
	{
		AddElement((LinkedListNode *) modRecord, &globalRecordList);
	}

	static ModuleRecord *create(char *modName, unsigned long modVersion, unsigned long type);
	static const ModuleRecord *search(char *name);
	static Symbol *querySymbol(char *symbolName, unsigned long& baseAddress);
private:
	static LinkedList globalRecordList kxhide;
};

}

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
ModuleRecord *MdCreateModule(char *moduleName, unsigned long moduleVersion, unsigned long moduleType);

/**
 * Function: MdLoadCoreModule
 *
 * Summary:
 * This function loads the 'core' module. This refers to the kernel-executable's module
 * record, which is required for keeping its symbol table in the kernel module list. Thus,
 * a separate record is kept for the micro-kernel and added to the ModuleList.
 *
 * @Version 1
 * @Since Circuit 2.03
 * @Author Shukant Pal
 */
static inline ModuleRecord *MdLoadCoreModule(void){
	return MdCreateModule((char*) "CORE", 200300, KMT_EXC_MODULE);
}


extern ObjectInfo *tKMOD_RECORD;
extern LinkedList LoadedModules;

#endif
