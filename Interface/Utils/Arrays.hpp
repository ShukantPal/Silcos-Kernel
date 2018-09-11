/**
 * @file Arrays.hpp
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
#ifndef MDFRWK_ARRAYS_HPP__
#define MDFRWK_ARRAYS_HPP__

#include <KERNEL.h>

class Arrays
{
public:
	static void* copyOf(const void *original,
			unsigned int newLength);
	static void copy(const void *org, void *dst,
			unsigned int copySize);
	static void copyFast(const void *org, void *dst,
			unsigned int copySize);
	static void copyFromBack(const void *org, void *dst,
			unsigned int copySize);
	static void copyFastFromBack(const void *org, void *dst,
					unsigned int copySize);
	static void invokeAll(void (**functorArray)(), size_t count);
private:
	Arrays();
};

#endif /* MDFRWK_ARRAYS_HXX_ */
