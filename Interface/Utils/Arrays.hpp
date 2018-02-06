/* @file: Arrays.hpp
 * @module: ModuleFramework (@kernel.silcos.mdfrwk)
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

class Arrays
{
public:
	static void* copyOf(const void *original, unsigned int newLength);
	static void copy(const void *org, void *dst, unsigned int copySize);

	/*
	 * Copies data from org to dst in the form of unsigned longs.
	 *
	 * @param org - original array
	 * @param dst - destination array
	 * @param copySize - bytes to copy from org to dst
	 * @author Shukant Pal
	 */
	static void copyFast(const void *org, void *dst,
					unsigned int copySize)
	{
		copySize /= sizeof(unsigned long);
		unsigned long *orgl = (unsigned long*) org;
		unsigned long *dstl = (unsigned long*) dst;
		while(copySize--)
			*(dstl++) = *(orgl++);
	}

	/*
	 * Copies data from org to dst going down the array.
	 *
	 * @param org - end of original array
	 * @param dst - end of destination array
	 * @param copySize - bytes to copy from org to dst
	 * @author Shukant Pal
	 */
	static void copyFromBack(const void *org, void *dst,
					unsigned int copySize)
	{
		const char *orgb = (const char*) org;
		char *dstb = (char *) dst;
		while(copySize--)
			*(dstb--) = *(orgb--);
	}

	/*
	 * Copies data from org to dst going down the array in the form of
	 * unsigned longs.
	 *
	 * @param org - end of original array
	 * @param dst - end of destination array
	 * @param copySize - bytes to copy from org to dst
	 * @author Shukant Pal
	 */
	static void copyFastFromBack(const void *org, void *dst,
					unsigned int copySize)
	{
		copySize /= sizeof(unsigned long);
		unsigned long *orgl = (unsigned long *) org;
		unsigned long *dstl = (unsigned long *) dst;

		while(copySize--)
			*(dstl--) = *(orgl--);
	}
private:
	Arrays();
};

#endif /* MDFRWK_ARRAYS_HXX_ */
