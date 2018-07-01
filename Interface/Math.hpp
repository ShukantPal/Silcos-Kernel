///
/// @file Math.hpp
///
/// Provides enumerous math utilties for comparision, arithemetic, and
/// much more. These functions are dependent on the hardware-platform, but
/// always have a "default" version, in case it wasn't implemented for
/// an architecture. These are not atomic, for atomic versions, see Atomic.hpp
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

#ifndef MDFRWK_MATH_HPP_
#define MDFRWK_MATH_HPP_

class Math
{
public:
	static inline unsigned long bitScanForward(unsigned long n)
	{
		unsigned long lowestBitSet;
#if defined(IA32) || defined(IA64)
		asm volatile("bsf %1, %0" : "=r"(lowestBitSet)
				: "r"(n), "r"(lowestBitSet));
#else
		lowestBitSet = 0;
		while(!((n >> 1) & 1))
			++(lowestBitSet);
#endif
		return (lowestBitSet);
	}

	static inline unsigned long bitScanReverse(unsigned long n)
	{
		unsigned long highestBitSet;
#if defined(IA32) || defined(IA64)
		asm volatile("bsr %1, %0" : "=r"(highestBitSet)
				: "r"(n), "r"(highestBitSet));
#else
		highestBitSet = 0;
		while(n >> 1)
			++(highestBitSet);
#endif
		return (highestBitSet);
	}

	static inline bool bitTest(unsigned char n, unsigned long idx)
	{
		return ((bool)(n >> idx));
	}

	static inline bool bitTest(unsigned short n, unsigned long idx)
	{
		return ((bool)(n >> idx));
	}

	static inline bool bitTest(unsigned long n, unsigned long idx)
	{
		return ((bool)(n >> idx));
	}

	static inline bool bitTestAndSet(unsigned long n, unsigned long idx)
	{
		bool isSet = bitTest(n, idx);
		n = (1 << idx);
		return (isSet);
	}

	static inline char min(char n1, char n2)
	{
		return ((n1 < n2) ? n1 : n2);
	}

	static inline unsigned char min(unsigned char n1, unsigned char n2)
	{
		return ((n1 < n2) ? n1 : n2);
	}

	static inline short min(short n1, short n2)
	{
		return ((n1 < n2) ? n1 : n2);
	}

	static inline unsigned short min(unsigned short n1, unsigned short n2)
	{
		return ((n1 < n2) ? n1 : n2);
	}

	static inline int min(int n1, int n2)
	{
		return ((n1 < n2) ? n1 : n2);
	}

	static inline unsigned int min(unsigned int n1, unsigned int n2)
	{
		return ((n1 < n2) ? n1 : n2);
	}

	static inline long min(long n1, long n2)
	{
		return ((n1 < n2) ? n1 : n2);
	}

	static inline unsigned long min(unsigned long n1, unsigned long n2)
	{
		return ((n1 < n2) ? n1 : n2);
	}

	static inline char max(char n1, char n2)
	{
		return ((n1 > n2) ? n1 : n2);
	}

	static inline unsigned char max(unsigned char n1, unsigned char n2)
	{
		return ((n1 > n2) ? n1 : n2);
	}

	static inline short max(short n1, short n2)
	{
		return ((n1 > n2) ? n1 : n2);
	}

	static inline unsigned short max(unsigned short n1, unsigned short n2)
	{
		return ((n1 > n2) ? n1 : n2);
	}

	static inline int max(int n1, int n2)
	{
		return ((n1 > n2) ? n1 : n2);
	}

	static inline unsigned int max(unsigned int n1, unsigned int n2)
	{
		return ((n1 > n2) ? n1 : n2);
	}

	static inline long max(long n1, long n2)
	{
		return ((n1 > n2) ? n1 : n2);
	}

	static inline unsigned long max(unsigned long n1, unsigned long n2)
	{
		return ((n1 > n2) ? n1 : n2);
	}
private:
	Math();
};

#endif/* Math.hpp */
