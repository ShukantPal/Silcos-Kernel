///
/// @file Object.hpp
/// @module ModuleFramework (kernel.silcos.mdfrwk)
///
/// Defines the Object class which the ultimate parent for generic
/// objects. It allows minimal interoperability between objects which
/// don't inherit virtually.
///
/// The use of AbstractObject is semantic and it should not be used for
/// inheritance.
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

#ifndef MODULES_MODULEFRAMEWORK_OBJECT_HXX_
#define MODULES_MODULEFRAMEWORK_OBJECT_HXX_

#include "Heap.hpp"
#include <TYPE.h>

class String;

///
/// The Object class is used to cast other types of object into a "generic"
/// class like the void * pointer in C.
///
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

///
/// Defines a independent class that inherits from Object. It can be
/// used to debug data structures.
///
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

///
/// The default 'new' operator provided by the ModuleFramework. It should not
/// be used in the KernelHost before linking. It uses the kernel heap to fetch
/// a block of memory with the given size.
///
/// @param alloc_size - amount of raw memory required
///
inline void* operator new(size_t alloc_size)
{
	return (kmalloc(alloc_size));
}

///
/// The default 'delete' operator provided by the ModuleFramework. It should
/// not be used in KernelHost before linking. It returns the memory to the
/// kernel heap.
///
/// @param kmal_ptr - pointer to the kmalloc'ed memory
/// @param alloc_size - size of the allocation, disregarded by underlying
/// 			kfree
///
inline void operator delete(void* kmal_ptr, size_t alloc_size)
{
	kfree(kmal_ptr);
}

///
/// The array 'new' operator used for allocating large amounts of memory
/// containing the same object. Uses the underlying kmalloc function.
///
/// @param alloc_size - total size of the array
///
inline void* operator new[](size_t alloc_size)
{
	return (kmalloc(alloc_size));
}

#endif/* Object.hxx */
