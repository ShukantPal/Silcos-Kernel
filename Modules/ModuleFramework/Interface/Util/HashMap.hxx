/**
 * File: HashMap.hxx
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

#ifndef MODULES_MODULEFRAMEWORK_INTERFACE_HASHMAP_HXX_
#define MODULES_MODULEFRAMEWORK_INTERFACE_HASHMAP_HXX_

#include "Functional.hxx"
#include "Heap.hxx"

namespace HashMap::Internal
{
	enum HmConfig
	{
		DEFAULT_INITIAL_CAPACITY = 32,
		MAXIMUM_CAPACITY = 1024,
		DEFAULT_LOAD_FACTOR = 75
	};
}

class HashMap
{
public:
	HashMap();
	HashMap(unsigned int loadFactor);
	HashMap(unsigned int loadFactor, unsigned int initialCapacity);
protected:
	struct Entry
	{
		Object& key;
		Object& value;
	};
};

#endif /* MODULES_MODULEFRAMEWORK_INTERFACE_HASHMAP_HXX_ */
