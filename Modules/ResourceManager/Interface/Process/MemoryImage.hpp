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

#include <Resource/ContextManager.hpp>
#include <Util/RBTree.hpp>

namespace Process
{

/**
 * Class: MemoryImage
 *
 * Summary:
 * Represent the address-space of a isolated process. This allows dynamic
 * libraries and remote-code.
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
	Resource::RegionInsertionResult insertRegion
			(unsigned long initialAddress, unsigned long pageCount,
			unsigned long cfgFlags, PAGE_ATTRIBUTES permissions);

	Resource::RegionRemovalResult removeRegion
				(unsigned long initialAddress,
				unsigned long pageCount, unsigned short typeId);

	unsigned long includeInRegion(unsigned long initialAddress,
					unsigned long addressExtension);

	Resource::MemorySection* validateRegion(unsigned long address);

	static MemoryImage* getImage();

	static MemoryImage* getImage(unsigned long code[2],
			unsigned long data[2], unsigned long bss[2],
				unsigned long stack[2]);

	static bool deleteImage(MemoryImage*);
protected:
	unsigned long codePages;
	unsigned long dataPages;
	unsigned long bssPages;
	unsigned long pinnedPages;
	unsigned long libraryCount;

	RBTree* sectionTree;

	Resource::MemorySection* findRegion(unsigned long address);
	unsigned long carveRegion(unsigned long address, unsigned long limit,
					Resource::MemorySection* parent);

	MemoryImage();
	MemoryImage(unsigned long code[2], unsigned long data[2],
			unsigned long bss[2], unsigned long stack[2]);
	~MemoryImage();
};

}

#endif/* Process/MemoryImage.hpp */
