/**
 * File: MemorySection.hpp
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
#ifndef MODULES_RESOURCEMANAGER_INTERFACE_RESOURCE_MEMORYSECTION_HPP_
#define MODULES_RESOURCEMANAGER_INTERFACE_RESOURCE_MEMORYSECTION_HPP_

#include <Memory/Pager.h>
#include <Memory/KMemoryManager.h>
#include <Util/LinkedList.h>
#include <Util/RBTree.hxx>

namespace Resource
{

/**
 * Struct: MemorySection
 *
 * Summary:
 * Represents a memory-region in the address-space of any resource-holder, and
 * is used particularly by ContextManager for managing these to keep track of
 * free/allocated regions.
 *
 * Functions:
 * MemorySection - initialize a section
 *
 * Changes:
 * # RBTree is used instead of AvlTree allowing seperation of nodes
 * # pageCount is added into the struct, reducing overhead of subtraction
 *
 * Version: 2.1
 * Author: Shukant Pal
 */
struct MemorySection
{
	union
	{
		LinkedListNode chainNode;// Used for linking in a list
		struct
		{
			MemorySection *next;
			MemorySection *last;
		};
	};
	unsigned long initialAddress;// Initial address of region (modulo 4-8k)
	unsigned long finalAddress;// Final address of region (modulo 4-8k)
	unsigned long pageCount;// Count of pages in the region
	union
	{
		unsigned long cfgFlags;// Flags for configuring the region
		struct
		{
			unsigned short typeId;// Type-identifier
			unsigned short ctlFlags;// Behaviour-defining flags
		};
	};
	PAGE_ATTRIBUTES pagerFlags;// Paging attributes

	MemorySection(unsigned long initialAddress, unsigned long pageCount,
			unsigned long cfgFlags, PAGE_ATTRIBUTES pagerFlags)
	{
		this->initialAddress = initialAddress;
		this->finalAddress = initialAddress + pageCount * KPGSIZE;
		this->pageCount = pageCount;
		this->cfgFlags = cfgFlags;
		this->pagerFlags = pagerFlags;
	}

	MemorySection(unsigned long initialAddress, unsigned long finalAddress,
			unsigned short typeId, unsigned short ctlFlags, PAGE_ATTRIBUTES pagerFlags)
	{
		this->initialAddress = initialAddress;
		this->finalAddress = finalAddress;
		this->pageCount = (finalAddress - initialAddress) >> KPGOFFSET;
		this->typeId = typeId;
		this->ctlFlags = ctlFlags;
		this->pagerFlags = pagerFlags;
	}
};

}

#endif/* Resource/MemorySection.hpp */
