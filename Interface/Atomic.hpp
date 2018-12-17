///
/// @file Atomic.hpp
///
/// This file provides high-level syntax atomic utility functions. You can
/// use them instead of directly writing asm.
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

#ifndef __ATOMIC_HPP__
#define __ATOMIC_HPP__

#include <TYPE.h>

class Atomic
{
public:
	static void add(unsigned char delta, unsigned char *ptr)
	{
#if defined(IA32) || defined(IA64)
		asm volatile("lock addb %1, %0" : "=m"(*ptr) : "r"(delta), "m"(*ptr));
#endif
	}

	static void add(unsigned short delta, unsigned short *ptr)
	{
#if defined(IA32) || defined(IA64)
		asm volatile("lock adds %1, %0" : "=m"(*ptr) : "r"(delta), "m"(*ptr));
#endif
	}

	static void add(unsigned int delta, unsigned int *ptr)
	{
#if defined(IA32) || defined(IA64)
		asm volatile("lock addl %1, %0" : "=m"(*ptr) : "r"(delta), "m"(*ptr));
#endif
	}

	static void add(unsigned long delta, unsigned long *ptr)
	{
#if defined(IA32)
		asm volatile("lock addl %1, %0" : "=m"(*ptr) : "r"(delta), "m"(*ptr));
#elif
		asm volatile("lock addq %1, %0" : "=m"(*ptr) : "r"(delta), "m"(*ptr));
#endif
	}

	static void dec(unsigned char *ptr)
	{
#if defined(IA32) || defined(IA64)
		asm volatile("lock decb %0" : "=m"(*ptr) : "m"(*ptr));
#endif
	}

	static void dec(unsigned short *ptr)
	{
#if defined(IA32) || defined(IA64)
		asm volatile("lock decs %0" : "=m"(*ptr) : "m"(*ptr));
#endif
	}

	static void dec(unsigned int *ptr)
	{
#if defined(IA32) || defined(IA64)
		asm volatile("lock decl %0" : "=m"(*ptr) : "m"(*ptr));
#endif
	}

	static void dec(unsigned long *ptr)
	{
#if defined(IA32)
		asm volatile("lock decl %0" : "=m"(*ptr) : "m"(*ptr));
#elif defined(IA64)
		asm volatile("lock decq %0" : "=m"(*ptr) : "m"(*ptr));
#endif
	}

	static void oR(unsigned int *ptr, unsigned int op)
	{
		asm volatile("lock or %1, %0" : "=m"(*ptr) : "r"(op), "m"(*ptr));
	}

	static void inc(unsigned char *ptr)
	{
#if defined(IA32) || defined(IA64)
		asm volatile("lock incb %0" : "=m"(*ptr) : "m"(*ptr));
#endif
	}

	static void inc(unsigned short *ptr)
	{
#if defined(IA32) || defined(IA64)
		asm volatile("lock incs %0" : "=m"(*ptr) : "m"(*ptr));
#endif
	}

	static void inc(unsigned int *ptr)
	{
#if defined(IA32) || defined(IA64)
		asm volatile("lock incl %0" : "=m"(*ptr) : "m"(*ptr));
#endif
	}

	static void inc(unsigned long *ptr)
	{
#if defined(IA32)
		asm volatile("lock incl %0" : "=m"(*ptr) : "m"(*ptr));
#elif defined(IA64)
		asm volatile("lock incq %0" : "=m"(*ptr) : "m"(*ptr));
#endif
	}

	static void sub(unsigned char delta, unsigned char *ptr)
	{
#if defined(IA32) || defined(IA64)
		asm volatile("lock subb %1, %0" : "=m"(*ptr) : "r"(delta), "m"(*ptr));
#endif
	}

	static void sub(unsigned short delta, unsigned short *ptr)
	{
#if defined(IA32) || defined(IA64)
		asm volatile("lock subs %1, %0" : "=m"(*ptr) : "r"(delta), "m"(*ptr));
#endif
	}

	static void sub(unsigned int delta, unsigned int *ptr)
	{
#if defined(IA32) || defined(IA64)
		asm volatile("lock subl %1, %0" : "=m"(*ptr) : "r"(delta), "m"(*ptr));
#endif
	}

	static void sub(unsigned long delta, unsigned long *ptr)
	{
#if defined(IA32)
		asm volatile("lock subl %1, %0" : "=m"(*ptr) : "r"(delta), "m"(*ptr));
#elif defined(IA64)
		asm volatile("lock subq %1, %0" : "=m"(*ptr) : "r"(delta), "m"(*ptr));
#endif
	}

	static void xchg(unsigned int val, unsigned int *ptr)
	{
#if defined(IA32) || defined(IA64)
		asm volatile("xchg %1, %0" : "=m"(*ptr) : "r"(val), "m"(*ptr));
#endif
	}

	static void xchg(unsigned long val, unsigned long *ptr)
	{
#if defined(IA32)
		asm volatile("xchg %1, %0" : "=m"(*ptr) : "r"(val), "m"(*ptr));
#elif defined(IA64)
		asm volatile("xchg %1, %0" : "=m"(*ptr) : "r"(val), "m"(*ptr));
#endif
	}
};

#endif/* Atomic.hpp */
