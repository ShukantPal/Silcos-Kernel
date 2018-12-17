/**
 * File: VolatileCache.hpp
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

#ifndef INTERFACE_MEMORY_VOLATILECACHE_HPP_
#define INTERFACE_MEMORY_VOLATILECACHE_HPP_

#include "../Utils/Stack.h"
#include "KObjectManager.h"

/**
 * Class: VolatileCache
 *
 * Summary:
 * Any subsystem which holds a cache of object to use should interface with its
 * allocator
 */
class VolatileCache
{
public:
	void import(void *object);
	void exportAll(unsigned long ocount);
	unsigned long totalSize();
private:
	ObjectInfo *type;
	Stack storage;
};

#endif/* Memory/VolatileObjectCache.hpp */
