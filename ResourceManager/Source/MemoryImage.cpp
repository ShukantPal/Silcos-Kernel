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
#include <KERNEL.h>
#include <Process/MemoryImage.hpp>

using namespace Resource;
using namespace Process;

static const char *nmMemoryImage = "Process::MemoryImage";
ObjectInfo *t_MemoryImage;

unsigned long NO_ENTRY = 0xDBDAFEFC;

/**
 * Function: MemoryImage::insertRegion
 *
 * Summary:
 * Inserts a memory-region into the address-space with the given bounds.
 *
 * Author: Shukant Pal
 */
RegionInsertionResult MemoryImage::insertRegion(unsigned long initialAddress, unsigned long pageCount,
							unsigned long cfgFlags, PAGE_ATTRIBUTES pageFlags)
{
	MemorySection *arena = new MemorySection(initialAddress, pageCount, cfgFlags, pageFlags);

	RegionInsertionResult chainOutput = ContextManager::add(arena);

	if(chainOutput != RegionInsertionResult::InsertSuccess)
		delete arena;

	if(chainOutput == RegionInsertionResult::Extension)
		DbgLine(" --extended");

	return (chainOutput);
}

/**
 * Function: MemoryImage::removeRegion
 *
 * Summary:
 * Finds all the regions in the given boundaries and removes all of them,
 * returning the number of arenas removed.
 *
 * Args:
 * unsigned long iaddr - the initial address of the region
 * unsigned long pageCount - no. of pages in the region (for finding faddr)
 * unsigned short typeId - type-id of the regions to removed, (Any) for all
 *
 * Returns: (@RegionRemovalResult)
 * the result of the operation -
 * # RegionCount() - get the no. of regions removed
 * # ErrRegionDoesntExist - no arena found
 *
 * Author: Shukant Pal
 */
RegionRemovalResult MemoryImage::removeRegion(unsigned long iaddr, unsigned long pageCount,
							unsigned short typeId)
{
	unsigned long faddr = iaddr + (pageCount << KPGOFFSET);
	MemorySection *first = NULL, *last = NULL;

	if(treePopulated)
	{
		first = (MemorySection*) arenaTree->getLowerBoundFor(iaddr);
		last = (MemorySection*) arenaTree->getLowerBoundFor(faddr);
	}
	else
	{
		MemorySection *tarena = firstArena;

		while(tarena != NULL)
		{
			if(tarena->finalAddress > iaddr)
			{
				first = tarena;
				break;
			}
			tarena = tarena->next;
		}

		while(tarena != NULL)
		{
			if(tarena->finalAddress > faddr)
			{
				last = tarena;
				break;
			}
			tarena = tarena->next;
		}

		if(last && last->initialAddress >= faddr)
			last = last->last;
	}

	unsigned long pcount = 0;// no. of pages removed
	unsigned long count = 0;

	/*
	 * Here, the first and last arenas in the chain are special cases as
	 * they may be partially present in the given region and may require
	 * splitting/carving. All others can be fully removed (if any)
	 */

	if(first)
	{
		if(first->initialAddress >= iaddr)
		{
			if(first->finalAddress <= faddr)
			{
				pcount += first->pageCount;
				remove(first);
				++(count);
			}
			else
			{
				pcount += (faddr - first->initialAddress) >> KPGOFFSET;
				carve(first, faddr, first->finalAddress);
			}
		}
		else if(first->initialAddress < iaddr)
		{
			if(first->finalAddress < faddr)
			{
				pcount += (iaddr - first->initialAddress) >> KPGOFFSET;
				carve(first, first->initialAddress, iaddr);
			}
			else
			{
				pcount += (faddr - iaddr) >> KPGOFFSET;
				split(first, iaddr, faddr);
			}
			++(count);
		}
	}

	if(last != first && last != first->next && last)
	{
		count = removeAll(first->next, last, typeId, pcount);
		MemorySection *rarena = first, *local;

		while(rarena != last)
		{
			local = rarena->next;
			delete rarena;
			rarena = local;
		}
	}

	if(last != first && last != NULL)
	{
		if(last->finalAddress <= faddr)
		{
			pcount += last->pageCount;
			remove(last);
		}
		else
		{
			pcount += (last->finalAddress - faddr) >> KPGOFFSET;
			carve(last, faddr, last->finalAddress);
		}
	}

	if(count)
	{
		return (RegionRemovalResult) RegionRemoval(count);
	}
	else
	{
		return (RegionRemovalResult::ErrRegionDoesntExist);
	}
}

void MemoryImage::init()
{
	t_MemoryImage = KiCreateType(nmMemoryImage, sizeof(MemoryImage), sizeof(long), NULL, NULL);
}

MemoryImage* MemoryImage::getImage()
{
	return new(t_MemoryImage) MemoryImage();
}

MemoryImage::MemoryImage() : ContextManager(nmMemoryImage, TypeId::psmmManager)
{
	this->code = this->data = this->bss = NULL;
	this->mainStack = NULL;
	this->pinnedPages = 0;
	this->libraryCount = 0;
}

MemoryImage::~MemoryImage()
{

}
