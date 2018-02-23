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
#ifndef RSMGR_MEMORY_SECTION_HPP__
#define RSMGR_MEMORY_SECTION_HPP__

#include <Memory/Pager.h>
#include <Memory/KMemoryManager.h>
#include <Utils/LinkedList.h>
#include <Utils/RBTree.hpp>

namespace Resource
{

#ifdef ARCH_32
	#define GetConfigFlags(type, ctl) (type | (ctl << 16))
#else
	#define GetConfigFlags(type, ctl) (type | (ctl << 32))
#endif

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
 * TODO: Create a internal subsystem in MemorySection for the management of stupid
 * 		memory-pages & their physical mappings. Mostly used for the future
 * 		of PFRA ______________________+++++.
 *
 * Version: 2.1
 * Author: Shukant Pal
 */
struct MemorySection
{
	enum Type
	{
		Any = NULL,
		Code = 0x1,
		Data = 0x2,
		BSS = 0x3,
		Stack = 0x4,
		Heap = 0x5,
		Shared = 0x6,
		Unknown = 0x7,
		Boundary = 0x8
	};

	MemorySection *next;
	MemorySection *last;
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

	MemorySection(){}// Dummy ctor (for useless purpose, nil)

	MemorySection(unsigned long initialAddress, unsigned long pageCount,
			unsigned long cfgFlags, PAGE_ATTRIBUTES pagerFlags) : next(NULL), last(NULL)
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
		this->next = this->last = NULL;
	}
};

}

#endif/* Resource/MemorySection.hpp */
