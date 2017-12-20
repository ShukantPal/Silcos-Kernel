/**
 * File: MemoryImage.hpp
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
#ifndef MEMORYIMAGE_HPP_
#define MEMORYIMAGE_HPP_

#include <Memory/KObjectManager.h>
#include <Resource/ContextManager.hpp>
#include <Util/RBTree.hxx>

extern ObjectInfo *tProcess_MemoryImage;

namespace Process
{

/**
 * Class: MemoryImage
 *
 * Summary:
 * Manages the address-space for a general process and uses a faster-BST when
 * the number of regions exceeds a limit.
 *
 * Function:
 * getImage - create a fresh image
 * deleteImage - try to dispose a address-space
 * findRegion - get the region containing a specific address
 * carveRegion - carve a child region from a larger region
 * MemoryImage - ctor for this
 *
 * Version: 1.2
 * Author: Shukant Pal
 */
class MemoryImage : public Resource::ContextManager
{
public:
	virtual Resource::RegionInsertionResult insertRegion(unsigned long initialAddress, unsigned long pageCount,
								unsigned long cfg, PAGE_ATTRIBUTES permissions);

	Resource::RegionRemovalResult removeRegion(unsigned long initialAddress, unsigned long pageCount,
							unsigned short typeId);

	unsigned long includeInRegion(unsigned long initialAddress, unsigned long addressExtension);

	Resource::MemorySection* validateRegion(unsigned long address);

	static MemoryImage* getImage();

	static MemoryImage* getImage(unsigned long code[2], unsigned long data[2], unsigned long bss[2],
					unsigned long stack[2]);

	static bool deleteImage(MemoryImage*);

	static void init();

	inline void dub()
	{
		arenaTree->_dub();
	}

protected:
	unsigned long pinnedPages;
	unsigned long libraryCount;
	unsigned long filterTable[8];// Keep track of count of all pages

	MemoryImage();
	MemoryImage(unsigned long code[2], unsigned long data[2],
			unsigned long bss[2], unsigned long stack[2]);
	~MemoryImage();

	virtual unsigned long *getIDFilter()
	{
		return (filterTable);
	}
};

}

#endif/* Process/MemoryImage.hpp */
