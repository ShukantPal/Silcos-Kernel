/**
 * File: Functional.hxx
 *
 * Summary:
 * Contains many functional interfaces
 *
 * Classes:
 * Hash - Contains a generic hashing-interface for most types used in the kernel. You
 * should specialize the Hash type, for your object and name the specializing
 * header "TypeHash.hxx".
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
#ifndef MDFRWK_FUNCTIONAL_HXX_
#define MDFRWK_FUNCTIONAL_HXX_

#include <String.hxx>

template<typename HashClient>
struct Hash
{
	unsigned int hashOf(HashClient& h)
	{
		return 0;
	}
};

template<>
struct Hash<String>
{
	static unsigned int hashOf(String& str)
	{
		return (str.hashCode());
	}
};

#endif /* MDFRWK_FUNCTIONAL_HXX_ */
