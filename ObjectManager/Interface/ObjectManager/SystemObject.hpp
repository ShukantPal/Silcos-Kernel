/**
 * File: SystemObject.hpp
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

#ifndef MODULES_OBJECTMANAGER_INTERFACE_OBJECTMANAGER_SYSTEMOBJECT_HPP_
#define MODULES_OBJECTMANAGER_INTERFACE_OBJECTMANAGER_SYSTEMOBJECT_HPP_

#include "Handle.hpp"
#include <String.hxx>

namespace ObjectManager
{

class SystemObject
{
public:
	virtual HANDLE open() = 0;
	virtual HANDLE duplicate(HANDLE) = 0;
	virtual SystemObject* parse(String& path) = 0;
	virtual void close(HANDLE) = 0;
protected:
	SystemObject();
	~SystemObject();
private:
	unsigned long referCount;
	unsigned long openHandles;
	String& localName;
	String& globalName;
	LinkedList openHandleList;
};

}

#endif/* ObjectManager/SystemObject.hpp */
