/**
 * File: SystemObject.hpp
 *
 * Summary:
 * Declares classes to abstract system-global objects & ways to share them with
 * other modules. It also provides security-mechanisms for user-mode clients.
 *
 * ___________________________________________________________________
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
#ifndef __OBJMGR_SYSTEMOBJECT_HPP__
#define __OBJMGR_SYSTEMOBJECT_HPP__

#include <Object.hxx>
#include <String.hxx>

namespace ObjectManager
{

class ObjectGenerator;
class Directory;
typedef unsigned long Handle;

enum ObjectPermissions
{
	READ = (1 << 0),
	WRITE = (1 << 1),
	COPY = (1 << 2),
	DESTROY = (1 << 3)
};

/**
 * Class: SystemObject
 *
 * Summary:
 * Provides a abstract interface to manage system-wide global objects. It has
 * a name & a global-path to reach it in the object-manager tree. Security enforcement
 * is provided by giving kernel software a interface to check whether the calling
 * process has access to this object.
 *
 * Functions:
 * getDirectory() - Get the parent folder/namespace for this object
 * getHandle() - Create a handle with given permissions for the object
 *
 * Author: Shukant Pal
 */
class SystemObject : public Object
{
public:
	Directory *getDirectory(){ return (parentFolder); }
	void getHandle(ObjectPermissions forAccess, Handle& handle);

	virtual unsigned int hashCode() = 0;
	virtual String& toString(){ return (globalName); }
protected:
	String localName;
	String globalName;
	Directory *parentFolder;
	int resourceOwner;
	SystemObject();
	SystemObject(String& locationPath, Directory *parentFolder = NULL);
	virtual ~SystemObject();
};

class Directory : public SystemObject
{
public:
	SystemObject& createObject(const char *localName, ObjectGenerator& generator);
	unsigned int hashCode();
private:
	Directory();
	LinkedList childList;
	// TODO: Implement string table for hashing all the children;
};

/**
 * Class: SystemTree
 *
 * Summary:
 * Complete overview of the object manager, provides interface to create, parse,
 * delete, etc. system-objects in a system-wide hierarchy of them.
 *
 * Functions:
 * createObject - create a object with a given path
 * parseObject - parse a path and find the object relevant
 * deleteObject - used for dereferencing objects
 *
 * Author: Shukant Pal
 */
class SystemTree final
{
public:
	static SystemObject& createObject(const char * locationPath, bool appendDirectories, ObjectGenerator& generator);
	static SystemObject *parseObject(const char *locationPath, bool createIfNone);
	static void deleteObject(SystemObject& objectInTree, bool deleteChildren = true, ObjectGenerator& generator);
	static bool deleteObject(const char *location, bool deleteChildren = true, ObjectGenerator& generator);
private:
	static Directory& root;
	// TODO: Implement string table for hashing all strings system-wide;
	SystemTree();
};

}

#endif/* SystemObject.hpp */
