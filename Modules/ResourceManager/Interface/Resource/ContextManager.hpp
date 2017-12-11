/**
 * File: ContextManager.hpp
 *
 * Summary:
 * This file introduces the concept of a context-manager which holds data about
 * regions in a resource-holder's address space. A address-space consists of
 * mapped and un-mapped portions, and to record whether a region is mapped or
 * not, which type of data is present in each space, or whether it is shared
 * memory is all done by the ContextManager. It is very useful for the PFRA
 * sub-system (@com.silcos.rsmgr).
 * 
 * Classes:
 * ContextManager - manages the context of a resource-holder
 *
 * -------------------------------------------------------------------
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 *
 * Copyright (C) 2017 - Shukant Pal
 */
#ifndef MODULES_RESOURCEMANAGER_INTERFACE_RESOURCE_CONTEXTMANAGER_HPP_
#define MODULES_RESOURCEMANAGER_INTERFACE_RESOURCE_CONTEXTMANAGER_HPP_

#include "MemorySection.hpp"
#include "Pager.h"
#include <Memory/KObjectManager.h>

namespace Resource
{

/**
 * Enum: RegionInsertionResults
 *
 * Summary:
 * Holds the return types that can be given by ContextManager.insertRegion()
 * allowing the caller to induce the change in the address-space.
 *
 * Macros:
 * IsInserted - Tells whether the address specified was mapped
 *
 * Version: 1.2
 * Author: Shukant Pal
 */
enum RegionInsertionResult
{
#define IsInserted(rt) (rt < 0xE0)
	InsertSuccess = 0xB1,		// A region was newly inserted
	Extension = 0xB2,		// A existing region was extended
	ErrRegionAlreadyExists = 0xE1,	// Already a region contains the bounds
	ErrResourceOverflow = 0xE2	// Resources are little to do insertion
};

/**
 * Enum: RegionRemovalResult
 *
 * Summary:
 * Holds the return types that can be given by ContextManager.removeRegion(),
 * allowing the caller to induce what occured.
 *
 * Macros -
 * RegionCount - No. of regions that were removed by the call
 * RegionRemoval - Used to get the return-value for cnt-number-of-region removed
 * IsRegionRemoved - true/false, if any region was removed by the call
 *
 * Author: Shukant Pal
 */
enum RegionRemovalResult
{
#define RegionCount(rt) (rt - 0xA1)
#define RegionRemoval(cnt) (0xA1 + cnt)
#define IsRegionRemoved(rt) (rt > 0xA1)
	ErrRegionDoesntExist = 0xA0 // The region didn't exist at all
};

/**
 * Class: ContextManager
 *
 * Summary:
 * Manages the address-space for a resource-holder like a process. It typically
 * stores regions in a data structure & provides the interface to manipulate
 * internal-mappings of memory.
 *
 * Functions:
 * insertRegion - insert a memory-section into the address-space
 * removeRegion - make sure a region of memory is not mapped
 * includeInRegion - extend a region allowing it to include a given address
 * validateRegion - validate & get the region holding a given address
 *
 * Version: 1.2
 * Since: Circuit 2.03
 */
class ContextManager
{
public:
/* Specifies the type for this context manager */
enum TypeId
{
	psmmManager = 0
};

	const char *name;
	const int typeId;

	virtual RegionInsertionResult insertRegion(unsigned long initialAddress,
			unsigned long pageCount, unsigned long cfgFlags,
			PAGE_ATTRIBUTES permissions) = 0;

	virtual RegionRemovalResult removeRegion(unsigned long initialAddress,
			unsigned long pageCount, unsigned short typeId) = 0;

	virtual unsigned long includeInRegion(unsigned long initialAddress,
			unsigned long addressExtension) = 0;

	virtual MemorySection* validateRegion(unsigned long address) = 0;

	static void init();
protected:
	MemorySection *code;
	MemorySection *data;
	MemorySection *bss;
	MemorySection *mainStack;
	MemorySection *recentCache;
	unsigned long referCount;

	static ObjectInfo *tMemorySection;

	ContextManager(const char *name, const int typeId);
	virtual ~ContextManager();
};

}

#endif/* Resource/ContextManager.hpp */
