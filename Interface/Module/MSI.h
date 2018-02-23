///
/// @file MSI.h
///
/// This file relates to elf-objects and the KernelHost. It is not for
/// message-signalled interrupts, please note.
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

#ifndef __INTERFACE_MODULE_MSI_H__
#define __INTERFACE_MODULE_MSI_H__

#include "Elf/ELF.h"
#include <Multiboot2.h>

typedef struct SectionHeader MSI_SHDR;

///
/// Limited KernelHost elf-object cache present for use in the KernelElf
/// class. It contains the information retrieved from multiboot and its
/// tables.
///
/// @version 1.0
/// @since Circuit 2.03
/// @author Shukant Pal
///
typedef
struct MultibootElfCache {
	U32 SectionHeaderCount;
	U32 SectionHeaderEntrySize;
	U32 SectionNameIndex;
	char *SectionNames;
	MSI_SHDR *SectionHeaderTable;
	struct {
		char *DynamicSymbolNames;
		struct Module::Elf::Symbol *DynamicSymbolTable;
		unsigned long DynamicSymbolCount;
	};
	struct Module::Elf::SymbolTable eSymbolTbl;
	struct Module::Elf::HashTable eSymbolHashTbl;
} KCOR_MSICACHE;

extern KCOR_MSICACHE msiKernelSections;

///
/// Specially designed class to interpret the KernelHost elf-object. This is
/// due to the fact that the boot-loader doesn't load the "file" for the
/// kernel-image. This class gets information present in the KernelHost
/// segments.
///
/// @version 1.0
/// @since Silcos 2.05
/// @author Shukant Pal
///
class KernelElf
{
public:
	static Module::Elf::DynamicEntry *getDynamicEntry(
					Module::Elf::DynamicTag tag) kxhide;
	static Module::ModuleRecord *registerDynamicLink() kxhide;
	static void loadBootModules() kxhide;
};
  
#endif/* Module/MSI.h */
