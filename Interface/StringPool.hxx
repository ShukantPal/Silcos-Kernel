/**
 * File: StringTable.hxx
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

#ifndef MDFRWK_STRINGPOOL_HXX_
#define MDFRWK_STRINGPOOL_HXX_

#include "String.hxx"
#include "Utils/LinkedList.h"

enum StringPoolConfig
{
	MAXIMUM_SIZE = 1024,
	DEFAULT_INITIAL_SIZE = 128,
	MINIMUM_SIZE = 16
};

struct StringEntry
{
	LinkedListNode entryLink;
	unsigned int hash;
	char *valueBuffer;
};

class StringPool
{
public:
	StringPool();
	StringPool(unsigned long capacity);
	String *getString(char *value);
	void disposeString(String *str);
private:
	unsigned long capacity;
	StringEntry *mapTable;
};

#endif/* ModuleFramework/StringTable.hxx */
