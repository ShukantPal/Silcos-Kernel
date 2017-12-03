/**
 * File: Math.hxx
 *
 * Summary:
 * 
 * Functions:
 *
 * Origin:
 *
 * Copyright (C) 2017 - Shukant Pal
 */

#ifndef MODULES_MODULEFRAMEWORK_INTERFACE_MATH_HXX_
#define MODULES_MODULEFRAMEWORK_INTERFACE_MATH_HXX_

class Math
{
public:
	static inline int min(int n1, int n2)
	{
		if(n1 < n2)
			return (n1);
		else
			return (n2);
	}

	static inline int max(int n1, int n2)
	{
		if(n1 > n2)
			return (n1);
		else
			return (n2);
	}
};

#endif /* MODULES_MODULEFRAMEWORK_INTERFACE_MATH_HXX_ */
