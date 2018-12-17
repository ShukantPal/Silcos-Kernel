/**
 * @file Arrays.cpp
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
#include <Utils/Arrays.hpp>
#include <Heap.hpp>
#include <Utils/Memory.h>

/**
 * Copies all the elements stored in <tt>org</tt> array to <tt>dst
 * </tt>, by reading and writing each byte at a time.
 *
 * @param org - source array
 * @param dst - destination buffer/array
 * @param copySize - size, in bytes, of the data to be copied
 */
void Arrays::copy(const void *org, void *dst, unsigned int copySize)
{
	const byte *orgPtr = (const byte*) org;
	byte *dstPtr = (byte*) dst;

	while(copySize--) {
		*(dstPtr++) = *(orgPtr++);
	}
}

/**
 * Copies all the elements stored in <tt>org</tt> array
 *
 * @param original - Memory from which the copy is to done
 * @param newLength - Length of the new array
 * @return - pointer to the newly copied array
 */
void* Arrays::copyOf(const void *org, unsigned int newLength)
{
	void *newCopy = kmalloc(newLength);
	copy(org, newCopy, newLength);
	return (newCopy);
}

/**
 * Copies all the elements of <tt>org</tt> array into the <tt>dst
 * </tt> buffer, reading and writing <tt>unsigned long</tt> data
 * at a time. This is generally faster than <tt>copy</tt>, but
 * is only useful for array having a size multiple of
 * <tt>sizeof(unsigned long)</tt>
 *
 * @param org - original array
 * @param dst - destination array
 * @param copySize - bytes to copy from org to dst
 */
void Arrays::copyFast(const void *org, void *dst,
		unsigned int copySize) {
	copySize /= sizeof(unsigned long);
	unsigned long *orgl = (unsigned long*) org;
	unsigned long *dstl = (unsigned long*) dst;

	while(copySize--) {
		*(dstl++) = *(orgl++);
	}
}

/**
 * Copies all the elements of <tt>org</tt> array into the <tt>dst
 * </tt> buffer, reading and writing one byte at a time, and
 * starting from the end. The <tt>org</tt> and <tt>dst</tt>
 * pointers must be backward, e.g. point to the end of their
 * respective memory buffers.
 *
 * @param org - end of original array
 * @param dst - end of destination array
 * @param copySize - bytes to copy from org to dst
 */
void Arrays::copyFromBack(const void *org, void *dst,
		unsigned int copySize) {
	const char *orgb = (const char*) org;
	char *dstb = (char *) dst;

	while(copySize--) {
		*(dstb--) = *(orgb--);
	}
}

/**
 * Copies all the elements of <tt>org</tt> array into the <tt>dst
 * </tt> buffer, reading and writing one <tt>unsigned long</tt>
 * at a time, and starting from the end. The <tt>org</tt> and
 * <tt>dst</tt> pointers must be backward, e.g. point to the end
 * of their respective memory buffers.
 *
 * @param org - end of original array
 * @param dst - end of destination array
 * @param copySize - bytes to copy from org to dst
 */
void Arrays::copyFastFromBack(const void *org, void *dst,
		unsigned int copySize) {
	copySize /= sizeof(unsigned long);
	unsigned long *orgl = (unsigned long *) org;
	unsigned long *dstl = (unsigned long *) dst;

	while(copySize--) {
		*(dstl--) = *(orgl--);
	}
}

/**
 * Invokes all functors present in the given array, serially, passing
 * no arguments.
 *
 * @param functorArray - pointer to functor array
 * @param count - number of functors to invoke
 */
void Arrays::invokeAll(void (**functorArray)(), size_t count)
{
	for(unsigned int funcIdx = 0; funcIdx < count; funcIdx++) {
		(*functorArray)();
		++(functorArray);
	}
}

