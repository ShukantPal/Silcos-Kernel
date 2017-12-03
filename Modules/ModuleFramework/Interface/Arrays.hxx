/**
 * File: Arrays.hxx
 *
 * Summary:
 * 
 * Functions:
 *
 * Origin:
 *
 * Copyright (C) 2017 - Shukant Pal
 */

#ifndef MODULES_MODULEFRAMEWORK_INTERFACE_ARRAYS_HXX_
#define MODULES_MODULEFRAMEWORK_INTERFACE_ARRAYS_HXX_

class Arrays
{
public:
	static void* copyOf(const void *original, unsigned int newLength);
	static void copy(const void *org, void *dst, unsigned int copySize);
private:
	Arrays();
};

#endif /* MODULES_MODULEFRAMEWORK_INTERFACE_ARRAYS_HXX_ */
