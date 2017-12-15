/**
 * File: MemoryImage.cpp
 *
 * Summary:
 * 
 * Functions:
 *
 * Origin:
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
#include <Process/MemoryImage.hpp>

using namespace Resource;
using namespace Process;

static const char *nmMemoryImage = "Process::MemoryImage";

unsigned long NO_ENTRY = 0xDBDAFEFC;

/**
 * Function: MemoryImage::insertRegion
 *
 * Summary:
 * Inserts a memory-region into the address-space with the given bounds.
 */
RegionInsertionResult MemoryImage::insertRegion(unsigned long initialAddress, unsigned long pageCount,
							unsigned long cfgFlags, PAGE_ATTRIBUTES pageFlags)
{
	MemorySection *newa = new(tMemorySection) MemorySection(initialAddress, pageCount, cfgFlags, pageFlags);
	RegionInsertionResult chainOutput = add(newa);

	if(chainOutput != RegionInsertionResult::InsertSuccess)
		kobj_free((kobj*) newa, tMemorySection);

	return (chainOutput);
}

MemoryImage::MemoryImage() : ContextManager(nmMemoryImage, TypeId::psmmManager)
{
	this->code = this->data = this->bss = NULL;
	this->codePages = this->dataPages = this->bssPages = 0;
	this->mainStack = NULL;
	this->pinnedPages = 0;
	this->libraryCount = 0;
	this->treeUsed = false;
}

MemoryImage::~MemoryImage()
{

}
