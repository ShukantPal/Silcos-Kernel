/**
 * File: HashMap.cxx
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
 * Copyright (C) 2017 - Shukant P
 */
#include <Memory/KObjectManager.h>
#include <Utils/HashMap.hpp>
#include <KERNEL.h>
#include <Utils/Arrays.hpp>

#define DEFAULT_INITIAL_CAPACITY 16
#define DEFAULT_LOAD_FACTOR 75
#define MAXIMUM_CAPACITY (1 << 28)

/* Slab-name for the hash-entry */
const char *nmHashMap_Entry = "HashMap::Entry";

/* Slab-allocator for allocating hash-entries */
ObjectInfo *tHashMap_Entry;

/* Slab-name for the hash-map */
const char *nmHashMap = "HashMap";

/* Slab-allocator for the hash-map itself */
ObjectInfo *tHashMap;

/* Used for NULL keys*/
class NullHash : public Object
{
public:
	unsigned int hashCode();
	NullHash();
};


NullHash::NullHash()
{

}

unsigned int NullHash::hashCode()
{
	return (0);
}

static NullHash nullHash;

/**
 * Function: HashMap::HashMap
 *
 * Summary:
 * Initializes the hash-map with all default values for the initial capacity
 * and the load-factor.
 *
 * Author: Shukant Pal
 */
HashMap::HashMap()
{
	this->capacity = DEFAULT_INITIAL_CAPACITY;
	this->loadFactor = DEFAULT_LOAD_FACTOR;
	this->threshold = calculateThreshold();
	this->bucketTable = (Entry**) kcalloc(sizeof(void*) * capacity);
	this->size = 0;
}

/**
 * Function: HashMap::HashMap
 *
 * Summary:
 * Initializes the hash-map with the initial capacity given and the default
 * load factor (75).
 *
 * Args:
 * unsigned long initialCapacity - the initial capacity for the hash-map
 *
 * Author: Shukant Pal
 */
HashMap::HashMap(unsigned long initialCapacity)
{
	if(initialCapacity > 128)
		initialCapacity = 128;
	else if(initialCapacity < 16)
		initialCapacity = 16;

	this->capacity = initialCapacity;
	this->loadFactor = DEFAULT_LOAD_FACTOR;
	this->threshold = calculateThreshold();
	this->bucketTable = (Entry**) kcalloc(sizeof(void*) * capacity);
	this->size = 0;
}

/**
 * Function: HashMap::HashMap
 *
 * Summary:
 * Initializes the hash-map with the given initial capacity and load factor,
 * fully customizing the mappings.
 *
 * Args:
 * unsigned long initialCapacity - the initial bucket-size for the map
 * unsigned long loadFactor - fraction by which threshold for expansion is
 * 				calculated
 *
 * Author: Shukant Pal
 */
HashMap::HashMap(unsigned long initialCapacity, unsigned long loadFactor)
{
	if(loadFactor < 25)
		loadFactor =  25;
	else if(loadFactor > 95)
		loadFactor = 95;

	if(initialCapacity > 128)
		initialCapacity = 128;
	else if(initialCapacity < 16)
		initialCapacity = 16;

	this->capacity = initialCapacity;
	this->loadFactor = loadFactor;
	this->threshold = calculateThreshold();
	this->bucketTable = (Entry**) kcalloc(sizeof(void*) * capacity);
	this->size = 0;
}

/**
 * Function: HashMap::get
 *
 * Summary:
 * Returns the value associated with key. If NULL is returned, it
 * means either the pair's value is NULL or the pair doesn't exist. To get the
 * difference use containsKey();
 *
 * Author: Shukant Pal
 */
void* HashMap::get(Object& key)
{
	Entry *ent = getEntry(key);
	return (ent != NULL) ? ent->getValue() : NULL;
}

/**
 * Function: HashMap::put
 *
 * Summary:
 * Puts the key-value pair into the table, unless it was already present, and
 * if so, the pair's old value is returned and the new-value is put in
 * place. Here, the hash-map is enlarged if the size overtakes the next
 * threshold.
 *
 * Args:
 * Object& key - key for the pair
 * void *value - value for the pair
 *
 * Author: Shukant Pal
 */
void* HashMap::put(Object& key, void *value)
{
	unsigned int hash = key.hashCode();
	Entry *ent = bucketTable[indexFor(hash)];
	while(ent != NULL)
	{
		if(ent->hashCode() == hash &&
				(&ent->getKey() == &key ||
						key.equals(&ent->getKey())))
		{
			void *oldValue = ent->getValue();
			ent->setValue(value);
			Dbg("__value_found");
			return (oldValue);
		}

		ent = ent->iterate();
	}

	ent = new(tHashMap_Entry) Entry(key, value);
	addEntry(ent);

	return (NULL);
}

void* HashMap::putForNullKey(void *value)
{
	return put(nullHash, value);
}

/**
 * Function: HashMap::remove
 *
 * Summary:
 * Removes the entry associated with the key and returns its value.
 *
 * Args:
 * Object& key - the key associated with the pair
 *
 * Author: Shukant Pal
 */
void* HashMap::remove(Object& key)
{
	Entry *pair = removeEntryForKey(key);
	kobj_free((kobj*) pair, tHashMap_Entry);
	return (pair != NULL) ? pair->getValue() : NULL;
}

void* HashMap::removeForNullKey()
{
	return remove(nullHash);
}

static bool isHashMapInitialized = false;

/**
 * Function: HashMap::init
 *
 * Summary:
 * Creates the allocator for the internal HashMap::Entry type.
 *
 * Author: Shukant Pal
 */
void HashMap::init()
{
	if(!isHashMapInitialized)
	{
		tHashMap_Entry = KiCreateType(nmHashMap_Entry, sizeof(HashMap::Entry), sizeof(unsigned long),
									NULL, NULL);

		tHashMap = KiCreateType(nmHashMap, sizeof(HashMap), sizeof(unsigned long),
									NULL, NULL);

		new(&nullHash) NullHash();
	}
}

bool HashMap::Entry::equals(Entry *other)
{
	if(other != NULL)
	{
		Object& k1 = other->getKey();
		Object& k2 = other->getKey();
		if(&k1 == &k2 || k1.hashCode() == k2.hashCode())
			return (true);
	}

	return (false);
}

/**
 * Function: HashMap::Entry::unlink
 *
 * Summary:
 * Unlinks this entry from the slot given and replaces itself (if it is the
 * first entry in the slot) with its successor.
 *
 * Args:
 * Entry **slot - the slot which owns this entry (before unlinking)
 *
 * Author: Shukant Pal
 */
void HashMap::Entry::unlink(Entry **slot)
{
	if(next)
		next->previous = previous;

	if(previous)
		previous->next = next;
	else
		*slot = next;
}

/**
 * Function: HashMap::addEntry
 *
 * Summary:
 * Links the entry to its slot & resizes the table if required.
 *
 * Args:
 * Entry* ent - entry to be added
 *
 * Author: Shukant Pal
 */
void HashMap::addEntry(Entry *ent)
{
	Entry **bucketSlot = bucketTable + indexFor(ent->hashCode());

	ent->link(*bucketSlot);
	*bucketSlot = ent;

	if(size++ > threshold)
	{
		resize(capacity * 2);
	}
}

/**
 * Function: HashMap::getEntry
 *
 * Summary:
 * Searches for the entry containing the given key
 *
 * Args:
 * Object& key - the key for the entry
 *
 * Author: Shukant Pal
 */
HashMap::Entry* HashMap::getEntry(Object& key)
{
	unsigned int hash = key.hashCode();
	Entry *ent = bucketTable[indexFor(key.hashCode())];
	while(ent != NULL)
	{
		if(ent->hashCode() == hash &&
				(&key == &ent->getKey() ||
						key.equals(&ent->getKey())))
		{
			return (ent);
		}

		ent = ent->iterate();
	}

	return (NULL);
}

/**
 * Function: HashMap::removeEntryForKey
 *
 * Summary:
 * Removes the entry associated with a key, and thus returns it unlinked and
 * undeleted.
 *
 * Args:
 * Object& key - the key assoicated with the entry
 *
 * Author: Shukant Pal
 */
HashMap::Entry* HashMap::removeEntryForKey(Object& key)
{
	unsigned long hash = key.hashCode();
	unsigned long slotIndex = indexFor(key.hashCode());
	Entry **slot = bucketTable + slotIndex;
	Entry *ent = *slot;
	Entry *tcache;
	while(ent)
	{
		if(ent->hashCode() == hash &&
				(&key == &ent->getKey() ||
						key.equals(&ent->getKey())))
		{
			--(size);

			tcache = ent;
			ent = ent->iterate();
			ent->unlink(slot);

			return (ent);
		}
		else
			ent = ent->iterate();
	}

	return (NULL);
}

/**
 * Function: HashMap::resize
 *
 * Summary:
 * Resizes the bucket-table unless memory constraints are reached.
 *
 * Args:
 * unsigned long newSize - size for the new bucket-table
 *
 * Author: Shukant Pal
 */
bool HashMap::resize(unsigned long newSize)
{
	Entry **oldTable = bucketTable;
	if(capacity == MAXIMUM_CAPACITY)
	{
		threshold = MAXIMUM_CAPACITY;
		return (false);
	}
	else
	{
		Entry **newTable = (Entry**) krcalloc(oldTable, newSize);
		if(newTable)
		{
			transferAll(newTable, newSize);
			bucketTable = newTable;
			return (true);
		}
		else
			return (false);
	}
}

/**
 * Function: HashMap::transfer
 *
 * Summary:
 * Transfers the entry from the old-slot to its proper location in its
 * slot.
 *
 * Args:
 * Entry *dislocEntry - the dislocated entry
 * Entry **oldSlot - the slot in which it exists (before transfer)
 *
 * Author: Shukant Pal
 */
void HashMap::transfer(Entry *dislocEntry, Entry **oldSlot)
{
	dislocEntry->unlink(oldSlot);

	Entry **newSlot = bucketTable + indexFor(dislocEntry->hashCode());
	dislocEntry->link(*newSlot);
	*newSlot = dislocEntry;
}

/**
 * Function: HashMap::transferAll
 *
 * Summary:
 * Transfers all entries to the new table, and if both are located at the same
 * location in memory, only wrongly located entries are moved. Otherwise, all
 * entries are moved. This saves the relocation times.
 *
 * Args:
 * Entry **newTable - the new-table in which entries are to be transferred
 * unsigned long newSize - size of the new-table
 *
 * Author: Shukant Pal
 */
void HashMap::transferAll(Entry **newTable, unsigned long newSize)
{
	unsigned long oldSize = capacity;
	capacity = newSize;

	Entry **src = bucketTable;
	Entry **slot = src;
	Entry *ent = NULL;
	Entry *copy;

	if(src == newTable)
	{
		for(unsigned long slotIndex = 0; slotIndex < oldSize; slotIndex++)
		{
			ent = *slot;
			while(ent)
			{
				if(indexFor(ent->hashCode()) != slotIndex)
				{
					copy = ent;
					ent = ent->iterate();
					transfer(copy, slot);
				}
				else
					ent = ent->iterate();
			}

			++(slot);
		}
	}
	else
	{
		for(unsigned long slotIndex = 0; slotIndex < oldSize; slot++)
		{
			ent = *slot;
			while(ent)
			{
				copy = ent;
				ent = ent->iterate();
				transfer(copy, slot);
			}

			++(slot);
		}

		kfree((void*) src);
	}

	threshold = calculateThreshold();
}
