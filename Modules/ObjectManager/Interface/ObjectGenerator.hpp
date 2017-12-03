/**
 * File: ObjectGenerator.hpp
 *
 * Summary:
 * Declares the ObjectGenerator class, used for allowing modules to implement a
 * object-generator to export their global data structures into the kernel-software
 * infrastructure & to user-mode services.
 * 
 * Classes:
 * ObjectGenerator - Used for generating a object
 *
 * Origin:
 * Used for allowing other modules to create objects exported by a modules using
 * its ObjectGenerator.
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
#ifndef __OBJMGR_OBJECTGENERATOR_HPP_
#define __OBJMGR_OBJECTGENERATOR_HPP_

#include "SystemObject.hpp"
#include <String.hxx>

namespace ObjectManager
{

class ObjectGenerator
{
public:
	virtual SystemObject& create() = 0;
	virtual void destroy(SystemObject&) = 0;
protected:
	ObjectGenerator();
	virtual ~ObjectGenerator();
};

}

#endif/* ObjectGenerator.hpp */
