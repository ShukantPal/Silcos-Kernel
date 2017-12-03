/**
 * File: SystemObject.cpp
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

#include <SystemObject.hpp>

using namespace ObjectManager;

static Directory __systemRoot;

/**
 * Function: SystemObject::SystemObject
 *
 * Summary:
 * Initializes the system object as a root-object and with the root-name ("??"). It
 * should not be used for normal objects.
 *
 * Author: Shukant Pal
 */
SystemObject::SystemObject()
{
	this->localName = new(tString) String("??");
	this->globalName = localName;
	this->parentFolder = NULL;
	this->resourceOwner = 0;
}

/**
 * Function: SystemObject::SystemObject
 *
 * Summary:
 * Initializes the system-object with the path given & the parent-folder given.
 *
 * NOTE: This is a private constructor used by createObject which validates that
 * the given path is free, and provides the parentFolder by reading the path.
 *
 * Args:
 * String& path - path in the object-tree for the new object
 * Directory *parentFolder - Parent directory for this object
 *
 * Author: Shukant Pal
 */
SystemObject::SystemObject(
		String& path,
		Directory *parentFolder
){
	unsigned int lastSlash = path.lastIndexOf('/');
	if(lastSlash == -1)
		lastSlash = 0;

	this->localName = path.substring(lastSlash, path.length() - 1);
	this->globalName = path;
	this->parentFolder = parentFolder;
	this->resourceOwner = 0;
}

SystemTree::SystemTree()
{
	this->root = __systemRoot;
}
