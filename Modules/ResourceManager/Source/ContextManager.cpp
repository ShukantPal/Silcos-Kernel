/**
 * File: ContextManager.cpp
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
#include <Memory/KMemoryManager.h>
#include <Resource/ContextManager.hpp>

using namespace Resource;

static const char *nmMemorySection = "MemorySection";
ObjectInfo *ContextManager::tMemorySection;

ContextManager::ContextManager(const char *mgrName, const int typeIdt) : name(mgrName), typeId(typeIdt)
{
	this->code = this->data = this->bss = this->mainStack = NULL;
	this->referCount = NULL;
	this->regionCount = 0;
	this->recentCache = NULL;
	this->firstArena = this->lastArena = NULL;
	this->arenaTree = NULL;
	this->treeUsed = false;
}

ContextManager::~ContextManager()
{

}

/**
 * Function: ContextManager::add
 *
 * Summary:
 * Adds the region given into the linked-chain and checks whether a overlapping
 * region conflicts with the new one. It also returns the proper-insertion
 * result and could also induce a extension of a existing region.
 *
 * If a existing region was extended, then the subclass should free the newArena
 * or keep a pointer to it.
 *
 * If the chain-list is long and a binary tree is available then, the function
 * could use the red-black tree instead of sorted-list to insert the region.
 *
 * Args:
 * MemorySection *newArena - the new-arena of memory to be inserted in the chain
 *
 * Returns: (#RegionInsertionResult)
 * @InsertSuccess - if no extension was done, and the region was inserted
 * @Extension - an existing region was extended to include the new-arena
 * @ErrRegionAlreadyExists - if another region is overlapping with this region
 *
 * FAQ:
 * Q. Why can't a overlapping region be extended anyway?
 *
 * A. Because, adding a new-region means that you would put data into that arena,
 * which could cause corruption. For example, if a process creates a shared-mem
 * region overlapping with another one then it would cause a potential bug in
 * the system.
 *
 * Author: Shukant Pal
 */
RegionInsertionResult ContextManager::add(MemorySection *newArena)
{
	unsigned long begin = newArena->initialAddress;
	unsigned long end = newArena->finalAddress;;
	unsigned long ncfg = newArena->cfgFlags;

	MemorySection *arenaBefore;
	MemorySection *arenaAfter;

	/*
	 * Here we are locating the arenas/regions before the one that is to
	 * be inserted by sorted list or a red-black tree.
	 */

	if(!treeUsed)
	{
		MemorySection *tsec = firstArena;
		MemorySection *arenaBfor = NULL;

		while(tsec != NULL)
		{
			if(tsec->initialAddress >= end)
			{
				arenaBefore = tsec->last;
				break;
			}

			tsec = tsec->next;
		}
	}
	else
	{
		arenaBefore = (MemorySection*) arenaTree->getLowerBoundFor(begin);
		arenaAfter = (MemorySection*) arenaTree->getUpperBoundFor(begin);
	}

	/*
	 * Here we really insert the new-arena into the chain/tree unless a
	 * extension is feasible.
	 */

	if((arenaBefore && arenaBefore->next != arenaAfter) ||
			(arenaAfter && arenaAfter->last != arenaBefore))
	{// Coding error detected here
		return (RegionInsertionResult::ErrResourceOverflow);
	}
	else if((arenaBefore && arenaBefore->finalAddress > begin) ||
			(arenaAfter && arenaAfter->initialAddress < end))
	{
		return (RegionInsertionResult::ErrRegionAlreadyExists);
	}
	else
	{
		bool extended = false;

		if(arenaBefore && arenaBefore->finalAddress == begin &&
				arenaBefore->cfgFlags == ncfg)
		{
			arenaBefore->finalAddress = end;
			arenaBefore->pageCount += (end - begin) >> KPGOFFSET;
			extended = true;
		}

		if(arenaAfter && arenaAfter->initialAddress == end)
		{
			if(extended)
			{
				arenaBefore->finalAddress = arenaAfter->finalAddress;
				arenaBefore->pageCount += arenaAfter->pageCount;

				arenaBefore->next = arenaAfter->next;
				if(arenaBefore->next)
					arenaBefore->next->last = arenaBefore;

				if(treeUsed)
				{
					arenaTree->remove(arenaAfter->initialAddress);
				}
				kobj_free((kobj*) arenaAfter, tMemorySection);
			}
			else
			{
				arenaTree->remove(arenaAfter->initialAddress);
				arenaAfter->initialAddress = begin;
				arenaAfter->pageCount += (end - begin) >> KPGOFFSET;
			}

			return (RegionInsertionResult::Extension);
		}
		else if(extended)
		{
			return (RegionInsertionResult::Extension);
		}

		newArena->last = arenaBefore;
		newArena->next = arenaAfter;

		if(arenaBefore)
			arenaBefore->next = newArena;
		else
			firstArena = newArena;

		if(arenaAfter)
			arenaAfter->last = newArena;
		else
			lastArena = newArena;

		++(regionCount);
		return (RegionInsertionResult::InsertSuccess);
	}
}

void ContextManager::init()
{
	tMemorySection = KiCreateType(nmMemorySection, sizeof(MemorySection), sizeof(long), NULL, NULL);
}
