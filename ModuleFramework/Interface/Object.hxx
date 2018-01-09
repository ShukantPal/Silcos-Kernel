/**
 * File: Object.hxx
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

#ifndef MODULES_MODULEFRAMEWORK_OBJECT_HXX_
#define MODULES_MODULEFRAMEWORK_OBJECT_HXX_

#include "Heap.hxx"

class String;

class Object
{
public:
	virtual unsigned int hashCode();
	virtual bool equals(Object *anotherObject);
	virtual String& toString();
	virtual ~Object();
protected:
	Object(){}
};

/* A constructible object */
class AbstractObject final : public Object
{
public:
	AbstractObject() : Object() {}
	~AbstractObject(){}
};

#define internal static
#define include_kobject(name) extern struct ObjectInfo * name;

typedef int (*Comparator)(void* m1, void* m2);
typedef long (*Hasher)(void *m);

inline void* operator new(unsigned int alloc_size)
{
	return kmalloc(alloc_size);
}

inline void operator delete(void* kmal_ptr, unsigned int alloc_size)
{
	kfree(kmal_ptr);
}

inline void* operator new[](unsigned int alloc_size)
{
	return kmalloc(alloc_size);
}

#endif/* Object.hxx */
