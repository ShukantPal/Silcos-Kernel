/**
 * File: HashMap.hxx
 *
 * Summary:
 * A general use hash-map is provided here which uses linked-lists and normal
 * hashing techniques to store key-value pairs
 *
 * Classes:
 * HashMap - the generalized hash-map implementation
 * HashMap::Entry - a entry in the hash-map
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

#include <Object.hxx>
#include <Functional.hxx>
#include <Heap.hxx>
#include <Memory/KObjectManager.h>
#include <Util/RBTree.hxx>

extern ObjectInfo *tHashMap_Entry;
extern ObjectInfo *tHashMap;

/**
 * Class: HashMap
 *
 * Summary:
 * Re-sizable hash-map with uses a general Object as a key and any void*
 * value. It internally uses a RBTree to maintain entries within each bucket
 * and the Object.hashCode() for hashing keys.
 *
 * Changes:
 * # Re-sizing when size exceeds the threshold
 * # Allow 'NULL' keys
 *
 * Version: 1.2
 * Since: MDFRWK 1.0
 * Author: Shukant Pal
 */
class HashMap
{
public:
	HashMap();
	HashMap(unsigned long loadFactor);
	HashMap(unsigned long loadFactor, unsigned long initialCapacity);

	bool containsKey(Object& key)
	{
		return (getEntry(key) == NULL);
	}

	void* get(Object& key);
	void* put(Object& key, void *value);
	void* putForNullKey(void *value);
	void* remove(Object& key);
	void* removeForNullKey();
	void* get(Object& key);
	static void init();
protected:

/**
 * Struct: HashMap::Entry
 *
 * Summary:
 * Represents a entry into a hash-table which is held by a bucket.
 *
 * Functions:
 * getKey() - get the key for the entry
 * getValue() - get the value associated with the key
 * setValue() - modify the value of the entry
 *
 * Version: 1.1
 * Since: HashMap 1.0
 * Author: Shukant Pal
 */
struct Entry
{
private:
	Object& key;
	void *value;
	unsigned int hash;
	Entry *next;
	Entry *previous;
public:
	Entry(
			Object& key_,
			void *val
	) : key(key_) {
		this->hash = key.hashCode();
		this->value = val;
		this->next = NULL;
		this->previous = NULL;
	}

	Entry(
			Object& key_,
			void *val,
			Entry *next
	) : key(key_){
		this->hash = key.hashCode();
		this->value = val;
		this->next = next;
		this->previous = NULL;

		next->previous = this;
	}

	inline unsigned int hashCode(){ return (hash); }
	inline Object& getKey(){ return (this->key); }
	inline void* getValue(){ return (this->value); }
	inline void setValue(void *newv){ value = newv; }

	inline Entry* iterate(){ return (next); }

	inline void link(Entry* slot)
	{
		this->next = slot;
		if(slot)
			slot->previous = this;
	}

	void unlink(Entry **slot);
	bool equals(Entry *other);
};// struct HashMap::Entry

	inline unsigned long calculateThreshold()
	{
		return (capacity * loadFactor / 100);
	}

	inline unsigned long indexFor(
			unsigned long hashv
	){
		return (hashv % capacity);
	}

	void addEntry(Entry *ent);
	Entry* getEntry(Object& key);
	Entry* removeEntryForKey(Object& key);
	bool resize(unsigned long size);

	unsigned long loadFactor;
	unsigned long capacity;
	unsigned long size;
	unsigned long threshold;
	Entry **bucketTable;

private:
	void transfer(Entry *dislocEntry, Entry **oldSlot);
	void transferAll(Entry **newTable, unsigned long newSize);
};

#endif/* Util/HashMap.hxx */
