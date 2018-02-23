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
#include <KERNEL.h>
#include <Resource/ContextManager.hpp>

using namespace Resource;

/**
 * Function: ContextManager::ContextManager
 *
 * Summary:
 * Initializes the object with all defaults values and requires the subclass
 * identity (name & typeId).
 *
 * Args:
 * const char *mgrName - name of the subclass
 * const int typeId - type id of the subclass
 *
 * Author: Shukant Pal
 */
ContextManager::ContextManager(const char *mgrName, const int typeIdt) : name(mgrName), typeId(typeIdt)
{
	this->code = this->data = this->bss = this->mainStack = NULL;
	this->referCount = 1;
	this->regionCount = 0;
	this->recentCache = NULL;
	this->firstArena = NULL;
	this->lastArena = NULL;
	this->arenaTree = NULL;
	this->treePopulated = false;
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

	MemorySection *arenaBefore = NULL;
	MemorySection *arenaAfter = NULL;

	/*
	 * Here we are locating the arenas/regions before the one that is to
	 * be inserted by sorted list or a red-black tree.
	 */

	if(!treePopulated)
	{
		MemorySection *tsec = firstArena;

		while(tsec != NULL)
		{
			if(tsec->initialAddress >= end)
			{
				arenaBefore = tsec->last;
				break;
			}

			tsec = tsec->next;
		}

		arenaAfter = tsec;
		if(!tsec)
		{
			if(lastArena)
				arenaBefore = lastArena;
			else
				arenaBefore = firstArena;
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

				if(treePopulated)
				{
					arenaTree->remove(arenaAfter->initialAddress);
				}
				delete arenaAfter;
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
		{
			arenaBefore->next = newArena;
		}
		else
		{
			firstArena = newArena;
		}

		if(arenaAfter)
		{
			arenaAfter->last = newArena;
		}
		else if(arenaBefore)
		{
			lastArena = newArena;
		}

		++(regionCount);

		/*
		 * Check if we could build a good-hold tree over here
		 */

		if(!treePopulated)
			populateTree();
		else
			arenaTree->insert(newArena->initialAddress, newArena);

		return (RegionInsertionResult::InsertSuccess);
	}
}

/**
 * Function: ContextManager::populateTree
 *
 * Summary:
 * Populates a new arena-tree for the image if not present, when the count of
 * arenas increases upto 32.
 *
 * Returns:
 * true, if a tree was newly allocated; false, if the tree was already present
 * or the count of arenas is small.
 *
 * Author: Shukant Pal
 */
bool ContextManager::populateTree()
{
	if(regionCount >= 32)
	{
		arenaTree = new(tRBTree) RBTree();
		MemorySection *arena = firstArena;

		while(arena != NULL)
		{
			arenaTree->insert(arena->initialAddress, arena);
			arena = arena->next;
		}

		treePopulated = true;
		return (true);
	}
	else
	{
		return (false);
	}
}

/**
 * Function: ContextManager::carve
 *
 * Summary:
 * Carves a another arena from the parent, by marking its initial & final
 * addresses to refreshed values. For this, iaddr & faddr must be within
 * bounds of the parent-arena. All placements (list & tree) are done
 * automatically here.
 *
 * Carving is done as follows -
 *
 * 		---------------------------------------------------------
 * 		|		|			|		|
 * 		+ LEFT-OVER	+	NEWLY		+ LEFT-OVER	+
 *		| SPACE (LEFT)	|	FORMED		| SPACE (RIGHT)	|
 *		+		+	ARENA		+		+
 *		|		|			|		|
 *		---------------------------------------------------------
 *				^			^
 *			      iaddr		      faddr
 * Args:
 * MemorySection *arena - the parent arena to carve
 * unsigned long iaddr, faddr - initial & final addresses respectively
 *
 * Author: Shukant Pal
 */
void ContextManager::carve(MemorySection *arena, unsigned long iaddr, unsigned long faddr)
{
	bool rinsert = treePopulated && (arena->initialAddress != iaddr);

	if(rinsert)
	{
		arenaTree->remove(arena->initialAddress);
	}

	/*
	 * Note that re-insertion of the arena is not necessary because the
	 * region is shrinking. (in the list, note)
	 */

	arena->initialAddress = iaddr;
	arena->finalAddress = faddr;
	arena->pageCount = (faddr - iaddr) >> KPGOFFSET;

	if(rinsert)
	{
		arenaTree->insert(iaddr, arena);
	}
}

/**
 * Function: ContextManager::split
 *
 * Summary:
 * Splits the parent into two regions by creating a hole (range - laddr to raddr)
 * as follows -
 *
 * 	-------------------------------------------------------------------------
 * 	|		|					|		|
 * 	|		|	DELETED HOLE FOR ARENA		|		|
 * 	|		|					|		|
 * 	-------------------------------------------------------------------------
 *	^		^					^		^
 * initial-addr	      laddr				      raddr        final-addr
 *
 * This function uses a simple replacement algorithm for splitting.
 *
 * Args:
 * MemorySection *arena - parent-arena
 * unsigned long laddr - left-bound for the hole
 * unsigned long raddr - right-bound for the hole
 *
 * Author: Shukant Pal
 */
void ContextManager::split(MemorySection *arena, unsigned long laddr, unsigned long raddr)
{
	unsigned long iaddr = arena->initialAddress;
	unsigned long faddr = arena->finalAddress;

	/*
	 * Here, there is no need to 'essentially' remove the parent as the
	 * iaddr (left-bound) arena has the same initial-address.
	 */

	arena->finalAddress = laddr;
	arena->pageCount = (laddr - iaddr) >> KPGOFFSET;

	MemorySection *rarena = new MemorySection(raddr, faddr, arena->typeId, arena->ctlFlags,
							arena->pagerFlags);
	if(treePopulated)
		arenaTree->insert(raddr, rarena);

	rarena->next = arena->next;
	rarena->last = arena;

	arena->next = rarena;
	if(rarena->next)
		rarena->next->last = rarena;
}

/**
 * Function: ContextManager::remove
 *
 * Summary:
 * Removes the arena-object from the list & tree allowing it to be cached or
 * freed back to memory.
 *
 * Args:
 * MemorySection *arena - arena to be removed
 *
 * Author: Shukant Pal
 */
void ContextManager::remove(MemorySection *arena)
{
	MemorySection *lower = arena->last;
	MemorySection *upper = arena->next;

	if(treePopulated)
		arenaTree->remove(arena->initialAddress);

	if(lower)
		lower->next = upper;
	else
		firstArena = upper;

	if(upper)
		upper->last = lower;
	else
		lastArena = lower;
}

/**
 * Function: ContextManager::removeAll
 *
 * Summary:
 * Removes all the arena in the range (from, till - 1) of the type-id given. If
 * type-id given is NULL then all are removed. All the removed regions are
 * linked like they were originally allowing the subclass to free or cache the
 * removed arenas.
 *
 * The first-arena->last & last-arena->next fields are undefined and have a
 * unique value on the stack. They should be stored as 'nil' values if iterating
 * the chain.
 *
 * Args:
 * MemorySection *from - first arena to remove
 * MemorySection *till - arena after the last arena to remove
 * unsigned short typeId - type of the regions to remove
 *
 * Returns:
 * the number of the regions that were unlinked from the main-chain of arena in
 * the context.
 *
 * Author: Shukant Pal
 */
unsigned long ContextManager::removeAll(MemorySection *from, MemorySection *till,
						unsigned short typeId, unsigned long& pageCount)
{
	unsigned long removalCount = 0;
	unsigned long *idFilt = getIDFilter();
	pageCount = 0;

	if(typeId == MemorySection::Type::Any)
	{
		if(treePopulated)
		{
			MemorySection *tarena = from;

			while(tarena != till)
			{
				if(tarena->typeId == typeId)
				{
					arenaTree->remove(tarena->initialAddress);

					pageCount += tarena->pageCount;
					idFilt[tarena->typeId] -= tarena->pageCount;
					++(removalCount);
				}

				tarena = tarena->next;
			}
		}

		MemorySection *before = from->last;
		MemorySection *after = till;

		if(before)
		{
			before->next = after;
		}
		else
		{
			firstArena = after;
		}

		if(after)
		{
			after->last = before;
		}
		else
		{
			lastArena = before;
		}
	}
	else
	{
		/*
		 * Here we try to keep the internal linkages of the removed
		 * subchain. Thus, we restore the forward & backward links
		 * after each removal.
		 */

		MemorySection nil;
		MemorySection *arena = from;// current-element being tested
		MemorySection *local = &nil;// last element inserted into sub-chain
		MemorySection *lcache;// cache for arena

		while(arena != till)
		{
			if(arena->typeId == typeId)
			{
				local->next = arena;
				lcache = arena->next;// more semantic, due to undef nature of remove

				remove(arena);
				arena->last = local;

				pageCount += (arena->pageCount);
				++(removalCount);

				arena = lcache;
			}
			else
				arena = arena->next;
		}

		local->next = &nil;
		idFilt[typeId] -= pageCount;
	}

	return (removalCount);
}

void ContextManager::printAll()
{
	MemorySection *arena = firstArena;
	while(arena != NULL)
	{
		Dbg("("); DbgInt(arena->initialAddress / (1024)); Dbg(", ");
			DbgInt(arena->pageCount); Dbg(")");
		arena = arena->next;
	}
}
