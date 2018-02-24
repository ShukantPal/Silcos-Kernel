///
/// @file ModuleLoader.hpp
/// @module KernelHost
///
/// Provides the required drivers & utilities to execute modules that
/// have been loaded in main memory.
/// -------------------------------------------------------------------
/// This program is free software: you can redistribute it and/or modify
/// it under the terms of the GNU General Public License as published by
/// the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program.  If not, see <http://www.gnu.org/licenses/>
///
/// Copyright (C) 2017 - Shukant Pal
///

#ifndef __INTERFACE_MODULE_MODULE_LOADER_H__
#define __INTERFACE_MODULE_MODULE_LOADER_H__

#include "ModuleRecord.h"
#include <Memory/Pager.h> // For PADDRESS type

void MdSetupLoader(void);

namespace Module
{

namespace Elf
{
	class ElfManager;
}

#define NO_ENTRY_ADDR 0xDBDAFEFC

///
/// Holds the information about a binary "blob" of just a executable
/// file in kernel-memory. It is passed around various methods while
/// loading & linking modules as a part of giving information and
/// bound-checking.
///
/// @version 2.1
/// @since Circuit 2.03
/// @author Shukant Pal
///
struct BlobRegister
{
	LinkedListNode liLinker;//!< Used while setting up modules in bundles
	PADDRESS loadAddr;//!< Physical address of the blob in main-memory
	unsigned long blobSize;//!< Size of the file/blob in main-memory
	unsigned long fileAddr;//!< Address at which the blob/file was mapped
	char *cmdLine;//!< Command-line arguments for this module
	ModuleRecord *regForm;//!< Sort of "registration" form of this module
	ABI abiFound;//!< Detected binary-interface for this module. Future
		     //! kernel may allow multiple binary-interfaces to
		     //! be inter-linked.
	Void *manager;//!< Generic pointer to the ABI-manager for the module,
		      //! ElfManager for elf-modules
};

///
/// Driver class for the module-loader which provides utility functions
/// to execute kernel-modules that have been copied into main-memory. Among
/// these, it also allows loading "bundles" of modules which may be
/// inter-dependent.
///
/// ToDo: Implement "domain" loading with each module having specific
/// symbol references. Used while UDI core services loads any driver.
///
/// @version 1.0
/// @since Circuit 2.03
/// @author Shukant Pal
///
class ModuleLoader
{
public:
	static void loadFile(BlobRegister &blob);
	static void loadBundle(LinkedList &blobList);
	static void init(BlobRegister &blob);
private:
	static void *moveFileIntoMemory(BlobRegister &blob) kxhide;
	static ABI globalizeDynamic(void *moduleMemory, ModuleRecord& kmRecord, BlobRegister &blob) kxhide;

	static void initElf(Elf::ElfManager &module);
public:
	static void linkFile(ABI binaryIfc, BlobRegister& blob) kxhide;
};

}

#endif/* Module/ModuleLoader.h */
