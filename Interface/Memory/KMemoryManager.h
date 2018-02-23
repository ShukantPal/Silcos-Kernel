///
/// @file KMemoryManager.h
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
#ifndef MEMORY_KMEMORYMANAGER_H
#define MEMORY_KMEMORYMANAGER_H

#include "Internal/BuddyAllocator.hpp"
#include "Internal/ZoneAllocator.hpp"
#include "Address.h"

#ifdef NAMESPACE_MAIN
	void SetupKMemoryManager(void) kxhide;
#endif

#ifdef NS_KMEMORYMANAGER
	//! Kernel-page for the given address
	#define KPG_AT(pgAddress)(KDYNAMIC + sizeof(KPAGE)* \
			(((unsigned long) pgAddress - KDYNAMIC) >> KPGOFFSET))

	//! Kernel-page residing at the given page-offset in dynamic memory
	#define KPGOPAGE(pgOffset)(KPAGE*)(KDYNAMIC + sizeof(KPAGE) * pgOffset)

	//! Virtual address of the given page
	#define KPGADDRESS(kpPtr)(KDYNAMIC + \
			(KPGSIZE * ((kpPtr-KDYNAMIC) / sizeof(KPAGE))))

	typedef
	struct _KPAGE {
		Memory::Internal::BuddyBlock BInfo;
		unsigned long HashCode;
	} KPAGE;
#endif

#define ZONE_KOBJECT 0
#define ZONE_KMODULE 1

#define MAXPGORDER 5
#define PGVECTORS BDSYS_VECTORS(MAXPGORDER)

ADDRESS KiPagesAllocate(unsigned long pgOrder, unsigned long prefZone,
		unsigned long pgFlags);
unsigned long KiPagesFree(ADDRESS pgAddress);
ADDRESS KiPagesExchange(ADDRESS pgAddress, unsigned long *status,
		unsigned long znFlags);

#define KiPageAllocate(prefZone) KiPagesAllocate(0, prefZone, FLG_NONE)
#define KiPageFree(pgAddress) KiPagesFree(pgAddress)

#endif/* Memory/KMemoryManager.h */
