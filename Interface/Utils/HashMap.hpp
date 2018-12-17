/**
 * @file HashMap.hpp
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
#ifndef MDFRWK_HASHMAP_HXX_
#define MDFRWK_HASHMAP_HXX_

#include <Functional.hxx>
#include <Memory/KObjectManager.h>
#include "../Heap.hpp"
#include "../Object.hpp"
#include "../Utils/RBTree.hpp"

extern ObjectInfo *tHashMap_Entry;
extern ObjectInfo *tHashMap;

/**
 * Flexible hash-map that uses <tt>Object</tt> keys and <tt>void *
 * </tt> values. Hash-codes are calculated using <tt>Object.hashCode
 * </tt>. Supports null keys by placing its own internal null-based
 * hash entry.
 */
class HashMap
{
public:
	HashMap();
	HashMap(unsigned long loadFactor);
	HashMap(unsigned long loadFactor, unsigned long initialCapacity);

	bool containsKey(Object& key) {
		return (getEntry(key) == NULL);
	}

	void* get(Object& key);
	void* put(Object& key, void *value);
	void* putForNullKey(void *value);
	void* remove(Object& key);
	void* removeForNullKey();
	static void init();
protected:
	struct Entry;

	inline unsigned long calculateThreshold() {
		return (capacity * loadFactor / 100);
	}

	inline unsigned long indexFor(unsigned long hashv) {
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

/**
 * Used by <tt>HashMap</tt> and its subclasses internally to hold
 * key-value pair entries. The <tt>key</tt> property is read-only
 * for an <tt>Entry</tt> object, but the <tt>value</tt> property
 * can be modified.
 */
struct HashMap::Entry
{
private:
	Object& key;
	void *value;
	unsigned int hash;
	Entry *next;
	Entry *previous;
public:
	Entry(Object& key_, void *val) : key(key_) {
		this->hash = key.hashCode();
		this->value = val;
		this->next = NULL;
		this->previous = NULL;
	}

	Entry(Object& key_, void *val, Entry *next) : key(key_) {
		this->hash = key.hashCode();
		this->value = val;
		this->next = next;
		this->previous = NULL;

		next->previous = this;
	}

	inline unsigned int hashCode() {
		return (hash);
	}

	inline Object& getKey() {
		return (this->key);
	}

	inline void* getValue() {
		return (this->value);
	}

	inline void setValue(void *newv) {
		value = newv;
	}

	inline Entry* iterate() {
		return (next);
	}

	inline void link(Entry* slot) {
		this->next = slot;
		if(slot)
			slot->previous = this;
	}

	void unlink(Entry **slot);
	bool equals(Entry *other);
};

#endif/* Utils/HashMap.hpp */
