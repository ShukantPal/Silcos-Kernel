/**
 * File: Math.hpp
 * Module: ModuleFramework (@kernel.silcos.mdfrwk)
 *
 * Summary:
 * Basic type-safe comparison functions for numeric types.
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

#ifndef MDFRWK_MATH_HPP_
#define MDFRWK_MATH_HPP_

class Math
{
public:
	static inline char min(char n1, char n2)
	{
		return (n1 < n2) ? n1 : n2;
	}

	static inline unsigned char min(unsigned char n1, unsigned char n2)
	{
		return (n1 < n2) ? n1 : n2;
	}

	static inline short min(short n1, short n2)
	{
		return (n1 < n2) ? n1 : n2;
	}

	static inline unsigned short min(unsigned short n1, unsigned short n2)
	{
		return (n1 < n2) ? n1 : n2;
	}

	static inline int min(int n1, int n2)
	{
		return (n1 < n2) ? n1 : n2;
	}

	static inline unsigned int min(unsigned int n1, unsigned int n2)
	{
		return (n1 < n2) ? n1 : n2;
	}

	static inline long min(long n1, long n2)
	{
		return (n1 < n2) ? n1 : n2;
	}

	static inline unsigned long min(unsigned long n1, unsigned long n2)
	{
		return (n1 < n2) ? n1 : n2;
	}

	static inline char max(char n1, char n2)
	{
		return (n1 > n2) ? n1 : n2;
	}

	static inline unsigned char max(unsigned char n1, unsigned char n2)
	{
		return (n1 > n2) ? n1 : n2;
	}

	static inline short max(short n1, short n2)
	{
		return (n1 > n2) ? n1 : n2;
	}

	static inline unsigned short max(unsigned short n1, unsigned short n2)
	{
		return (n1 > n2) ? n1 : n2;
	}

	static inline int max(int n1, int n2)
	{
		return (n1 > n2) ? n1 : n2;
	}

	static inline unsigned int max(unsigned int n1, unsigned int n2)
	{
		return (n1 > n2) ? n1 : n2;
	}

	static inline long max(long n1, long n2)
	{
		return (n1 > n2) ? n1 : n2;
	}

	static inline unsigned long max(unsigned long n1, unsigned long n2)
	{
		return (n1 > n2) ? n1 : n2;
	}
private:
	Math();
};

#endif/* Math.hpp */
