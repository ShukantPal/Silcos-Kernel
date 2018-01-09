/**
 * File: Integer.hxx
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

#ifndef MODULES_MODULEFRAMEWORK_INTERFACE_INTEGER_HXX_
#define MODULES_MODULEFRAMEWORK_INTERFACE_INTEGER_HXX_

#include <KERNEL.h>

class Integer final
{
public:
	static unsigned int hashCode(unsigned long x)
	{
#ifdef ARCH32
		x = ((x >> 16) ^ x) * 0x45d9f3b;
		x = ((x >> 16) ^ x) * 0x45d9f3b;
		x = (x >> 16) ^ x;
		return x;
#else
		x = (x ^ (x >> 30)) * (0xbf58476d1ce4e5b9);
		x = (x ^ (x >> 27)) * (0x94d049bb133111eb);
		x = x ^ (x >> 31);
		return x;
#endif
	}

private:
	Integer();
};

#endif /* MODULES_MODULEFRAMEWORK_INTERFACE_INTEGER_HXX_ */
