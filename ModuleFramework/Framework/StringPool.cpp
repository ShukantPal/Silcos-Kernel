/**
 * File: StringPool.cxx
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
#include <StringPool.hxx>
#include "../../Interface/Heap.hpp"

StringPool::StringPool()
{
	this->capacity = StringPoolConfig::DEFAULT_INITIAL_SIZE;
	this->mapTable = (StringEntry*) kmalloc(sizeof(StringEntry) * capacity);
}

StringPool::StringPool(
		unsigned long capacity
){
	this->capacity = capacity;
	this->mapTable = (StringEntry*) kmalloc(sizeof(StringEntry) * capacity);
}
