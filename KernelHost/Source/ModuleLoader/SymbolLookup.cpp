/**
 * @file SymbolLookup.cpp
 *
 * Implements the driver for fast lookups of system-wide symbols. Kernel
 * modules are also priveleged to dynamically create and delete symbols
 * no a need basis.
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
#include <Memory/KObjectManager.h>
#include <Module/ModuleContainer.hpp>
#include <Module/ModuleRecord.h>
#include <Module/SymbolLookup.hpp>
#include <Heap.hpp>

using namespace Module;

#define BASE_CAPACITY 1024
#define LOAD_FACTOR 50
#define MAXIMUM_SIZE 16 * 1024

SymbolLookup *Module::globalKernelSymbolTable;

/**
 * Initializes the symbolic-lookup table, and allocates brand-new
 * buckets.
 *
 * @version 1.0
 * @author Shukant Pal
 */
SymbolLookup::SymbolLookup()
{
	this->internalCapacity = BASE_CAPACITY;
	this->currentThreshold = (internalCapacity * LOAD_FACTOR) / 100;
	this->totalSymbols = 0;
	this->lookupBuckets = (SymbolicDefinition**)
			kcalloc(BASE_CAPACITY * sizeof(void*));
}

SymbolLookup::~SymbolLookup()
{

}

/**
 * Adds the orphaned symbol into the pool, setting its owner as null. This
 * is helpful while dumping all the boot-time kernel module symbols, as it
 * has no meaning to store the owner of each symbol - cause boot-time modules
 * won't be unloaded.
 *
 * @param value
 * @param name
 */
void SymbolLookup::add(unsigned long value, char *name)
{
	rwl.enterAsWriter();
	ensureCapacity(totalSymbols++);
	addDirect(value, OTHER, (unsigned char *) name, null);
	rwl.exitAsWriter();
}

/**
 * Adds a symbolic-definition from the given Elf-symbol linking it to
 * the given module. If the symbol is found as reference, it isn't added
 * at all.
 *
 * @param esym - symbol-entry in the elf-object
 * @param mhdl - handle to the owner module
 * @version 1.0
 * @since Silcos 3.02
 * @author Shukant Pal
 */
void SymbolLookup::add(Elf::Symbol& esym, char *nameTable,
		ModuleContainer *mcont)
{
	if(esym.value == 0)
		return;

	rwl.enterAsWriter();
	ensureCapacity(totalSymbols++);
	addDirect(esym.value + mcont->getBase(), OTHER,
			(unsigned char*) nameTable + esym.name, mcont);
	rwl.exitAsWriter();
}

/**
 * Adds all the symbolic definitions in the given table, if they have a
 * valid value (are definitions, not references).
 *
 * @param sTable - table of symbols in the elf-object
 * @param mcont - container for the module
 */
void SymbolLookup::addAll(Elf::SymbolTable& sTable, ModuleContainer *mcont)
{
	rwl.enterAsWriter();
	ensureCapacity(totalSymbols + sTable.entryCount);

	unsigned char *nameTable = (unsigned char*) sTable.nameTable;
	Elf::Symbol *esym = sTable.entryTable - 1;

	for(unsigned long sidx = 0; sidx < sTable.entryCount; sidx++)
	{
		++(esym);

		if(esym->value == 0)
			continue;

		addDirect(esym->value + mcont->getBase(), OTHER,
				(unsigned char*) nameTable + esym->name,
				mcont);
	}

	rwl.exitAsWriter();
}

/**
 * Returns the value of the symbol defined by the given name, if found, and
 * the defining modules container-object.
 *
 * @param symbolName[in] - the name of the symbol to lookup
 * @param owner[out] - the container-object for the defining module.
 */
unsigned long SymbolLookup::lookup(char *symbolName, ModuleContainer* &owner)
{
	rwl.enterAsReader();
	SymbolicDefinition *sdef = lookupBuckets[hashKey((unsigned char*)
			symbolName) % internalCapacity];

	while(sdef)
	{
		if(strcmp((const char*) symbolName, (const char*) sdef->name))
		{
			if(owner)
				owner = sdef->sandBox;
			return (sdef->address);
		}

		sdef = sdef->next;
	}

	rwl.exitAsReader();
	return (null);
}

/**
 * Ensures that the number of buckets in the hash-table are greater than or
 * equal to newSize, unless the maximum capacity of 16,384 has already been
 * reached.
 *
 * This allows faster lookups and deletions from the table. This occurs
 * according the LOAD_FACTOR that can be configured by changing the
 * SymbolLookup source file.
 *
 * @param newSize - the number of elements this table (will) be holding
 * @version 1.0
 * @since Silcos 3.02
 * @author Shukant Pal
 */
void SymbolLookup::ensureCapacity(unsigned long lowerBound)
{
	if(lowerBound > currentThreshold && lowerBound < MAXIMUM_SIZE)
	{
		SymbolicDefinition **newBuckets = (SymbolicDefinition **)
				krcalloc(lookupBuckets, internalCapacity * 2 *
						sizeof(unsigned long));
		if(newBuckets)
			moveAll(newBuckets, internalCapacity * 2);
	}
}

/**
 * Directly accesses the buckets and inserts the symbolic-definition without
 * any checking, other than an already existing definition.
 *
 * @param address[in] - Value of the symbol in the kernel space
 * @param type[in] - (Optional) type of the symbol
 * @param name[in] - Name of symbol by definition
 * @param mcont[in] - Owner module
 * @return - whether the symbol was added successfully; if the same symbol is
 * 		already defined, than the new one isn't added;
 */
bool SymbolLookup::addDirect(unsigned long address, SymbolType type,
		unsigned char *name, ModuleContainer *mcont)
{
	unsigned long nlen;
	unsigned long bidx = hashKey(name, nlen) % internalCapacity;

	for(SymbolicDefinition *oldSym = lookupBuckets[bidx]; oldSym != null;
			oldSym = oldSym->next)
	{
		if(strcmp((const char *) name, (const char *) &oldSym->name))
			return (false);
	}

	SymbolicDefinition *newSym = (SymbolicDefinition *) kmalloc(
			sizeof(SymbolicDefinition) + nlen + sizeof(char)
	);

	newSym->address = address;
	newSym->type = type;
	newSym->sandBox = mcont;
	memcpy(name, &newSym->name, nlen + 1);

	newSym->next = lookupBuckets[bidx];
	newSym->last = null;
	lookupBuckets[bidx] = newSym;

	return (true);
}


/**
 * Moves the given symbolic-definition from the old
 * bucket to the new bucket. This is done when resizing the hash
 * table.
 *
 * @param symdef - ptr to symbolic definition
 * @param oldBucket - the bucket in which the symbol already is
 * 			located
 * @param newBucket - the bucket in which the symbol should be moved
 * 			to.
 */
void SymbolLookup::move(SymbolicDefinition *symdef,
		SymbolicDefinition **oldBucket, SymbolicDefinition **newBucket)
{
	if(symdef->next)
		symdef->next->last = symdef->last;

	if(symdef->last)
		symdef->last->next = symdef->next;
	else
		*oldBucket = null;

	symdef->next = *newBucket;
	symdef->last = null;
	*newBucket = symdef;
}

/**
 * Moves all the symbolic definitions to the new buckets given. If the new
 * buckets are the same as the old ones, then symbols are conditionally
 * moved, based on whether there bucket-index is correct. Otherwise, all of
 * them are moved unconditionally.
 *
 * The first one gives better performance as copying overhead is minimal. It
 * depends upon krcalloc.
 *
 * @param newBuckets - the new buckets to use in the hash-table
 * @param newSize
 */
void SymbolLookup::moveAll(SymbolicDefinition **newBuckets,
		unsigned long newSize)
{
	DbgLine("CAME TO MOVE ME");
	SymbolicDefinition **oldBucket = lookupBuckets;
	SymbolicDefinition *anyElem, *nextInChain;
	unsigned long newIndex;

	if(newBuckets == lookupBuckets)
	{
		DbgLine("EXPANSION & MOVE ");
		for(unsigned long oldIndex = 0; oldIndex < internalCapacity;
				oldIndex++)
		{
			anyElem = *oldBucket;

			while(anyElem != null)
			{
				newIndex = newIndexFor(anyElem, newSize);
				nextInChain = anyElem->next;

				if(newIndex != oldIndex)
					move(anyElem, oldBucket,
							newBuckets + newIndex);

				anyElem = nextInChain;
			}

			++(oldBucket);
		}
	}
	else
	{
		for(unsigned long oldIndex = 0; oldIndex < internalCapacity;
				oldIndex++)
		{
			anyElem = *oldBucket;

			while(anyElem != null)
			{
				newIndex = newIndexFor(anyElem, newSize);
				nextInChain = anyElem->next;
				move(anyElem, oldBucket,
						newBuckets + newIndex);
				anyElem = nextInChain;
			}

			++(oldBucket);
		}

		kfree(lookupBuckets);
		lookupBuckets = newBuckets;
	}

	internalCapacity = newSize;
	currentThreshold = (internalCapacity * LOAD_FACTOR) / 100;
}

/**
 * Calculates the symbol-name's hash code that will be used to locate
 * and store any definitions its bucket. Currently, the GNU hash
 * algorithm is being used.
 *
 * @param name - name of the symbol being hashed
 * @since Silcos 3.02
 * @author Shukant Pal
 */
unsigned long SymbolLookup::hashKey(unsigned char *name)
{
	unsigned long hashCode = 5381;

	for(; *name; name++)
		hashCode += (hashCode << 5) + *name;

	return (hashCode);
}

/**
 * Calculates the symbol-name's hash code and also returns its length.
 *
 * @param name - name of the symbol being hashed
 * @param count[out] - length of the name in chars
 * @since Silcos 3.02
 * @author Shukant Pal
 */
unsigned long SymbolLookup::hashKey(unsigned char *name, unsigned long& count)
{
	unsigned long hashCode = 5381;
	unsigned char *nameStart = name;

	for(; *name; name++)
		hashCode += (hashCode << 5) + *name;

	count = name - nameStart;
	return (hashCode);
}
