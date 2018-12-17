/**
 * @file Namespace.cpp
 *
 * Implements the Namespace object which allows multiple modules to be kept
 * into groups. This features stuff like restricted symbolic linking, internal
 * weak references, encapsulation, prevention of symbol clashes, security,
 * prevention of page-faults due to wrong symbols, etc.
 *
 * A namespace on its own is immaterial (except for `root`). Speaking
 * generally, namespaces are themselves a parent kernel-module like the
 * in-kernel UDI environment.
 *
 * FutureGoals:
 *
 * * The Namespace tree may be further extended by BinaryObjects used by
 * modules to communicate in specific ways (interface/implementation).
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
#include <Module/Namespace.hpp>
#include <Utils/Memory.h>
#include <KERNEL.h>

using namespace Module;/* This is the C++ namespace :-) */

static const char *nsRootLocalName = "";
static const char *nsRootGlobalName = "::";
const ::Module::Namespace Namespace::nsRoot;

/**
 * Default constructor for private use only, by root-of-tree
 * namespaces.
 *
 * @version 1.0
 * @since Silcos 3.05
 * @author Shukant Pal
 */
Namespace::Namespace()
{
	//
	// This private constructor is intended only for building global
	// objects of internal use only, like nsRoot.
	//

	fileName = static_cast<char *>(null);
	nsLocalIdentifier = const_cast<char *>(nsRootLocalName);
	nsGlobalIdentifier = const_cast<char *>(nsRootGlobalName);
	nsOwner = static_cast<Namespace *>(null);
	totalChildren = 0;
}

/**
 * Constructs the namespace object with no associated file-name and with the
 * minimum constraints.
 *
 * This constructor can be accessed using the ns->create() function which
 * ensures no conflicts arise.
 *
 * @param name - local identifier for the namespace
 * @param owner - parent namespace for `this`
 * @since Silcos 3.05
 * @author Shukant Pal
 */
Namespace::Namespace(const char *name, Namespace *owner)
{
	unsigned long lIdentSize = strlen(name);
	unsigned long pgIdentSize = (owner != null) ?
			strlen(owner->nsGlobalIdentifier) + 2 : 2;

	fileName = (const char *) null;
	nsLocalIdentifier = (const char *) kmalloc(lIdentSize);
	nsGlobalIdentifier = (const char *) kmalloc(pgIdentSize + lIdentSize);
	nsOwner = owner;
	totalChildren = 0;
	strcpy(name, const_cast<char*>(nsLocalIdentifier));

	char *gIdentBuffer = const_cast<char*>(nsGlobalIdentifier);
	if(nsOwner)
		strcpy(owner->nsGlobalIdentifier, gIdentBuffer);
	gIdentBuffer += pgIdentSize - 2;
	*(gIdentBuffer++) = ':';
	*(gIdentBuffer++) = ':';
	strcpy(nsLocalIdentifier, gIdentBuffer);
}

/**
 * Constructs the namespace object with the associated file-name and local
 * namespace identifier with minimal constraints. This file name is usually
 * the build-time object-file name.
 *
 * This constructor can be acccessed by the ns->create() function which
 * ensures no conflicts occur.
 *
 * @param fromFile - name of the object-file/any-associated-file of the module
 * @param name - local identifier for the namespace
 * @param owner - parent of this namespace
 * @version 1.0
 * @since Silcos 3.05
 * @author Shukant Pal
 */
Namespace::Namespace(const char *fromFile, const char *name, Namespace *owner)
{
	unsigned long lIdentSize = strlen(name);
	unsigned long pgIdentSize = (owner != null) ?
			strlen(owner->nsGlobalIdentifier) : 0;

	fileName = (const char *) kmalloc(strlen(fromFile));
	nsLocalIdentifier = (const char *) kmalloc(lIdentSize);
	nsGlobalIdentifier = (const char *) kmalloc(pgIdentSize + lIdentSize);
	nsOwner = owner;
	totalChildren = 0;
	strcpy(fromFile, const_cast<char*>(fileName));
	strcpy(name, const_cast<char*>(nsLocalIdentifier));

	char *gIdentBuffer = const_cast<char*>(nsGlobalIdentifier);
	if(nsOwner)
		strcpy(owner->nsGlobalIdentifier, gIdentBuffer);
	gIdentBuffer += pgIdentSize;
	strcpy(nsLocalIdentifier, gIdentBuffer);
}

/**
 * Searches for the namespace with the given local name which is already
 * a direct child of this namespace.
 *
 * @param dChildIdentBuffer - buffer containing the name of the required
 * 		namespace. This may or may not be null-terminated, depending
 * 		on its source.
 * @param bytes - the no. of bytes in the buffer holding containing the local
 * 		identifier of the required naemspace.
 * @return - a pointer to the namespace, holding the given local identifier,
 * 		if found; otherwise, a null-pointer is returned on exit.
 */
Namespace *Namespace::searchOnly(const char *dChildIdentBuffer,
		unsigned bytes)
{
	for(Namespace *dChild = static_cast<Namespace *>(nsChildSet.head);
			dChild != null;
			dChild = static_cast<Namespace *>(dChild->next))
	{
		if(strcmpn(dChildIdentBuffer, dChild->nsLocalIdentifier,
				bytes))
			return (dChild);
	}

	return (null);
}

/**
 * Searches for a namespace with the identifier that is relative to this
 * namespace. For example, "::UDI::Network::Wireless" is a global identifier.
 * But w.r.t the UDI namespace, the "Network" identifier is relative.
 * ::HAL\0
 * @param dChildIdentBuffer
 * @return
 */
Namespace *Namespace::searchRelative(const char *dChildIdentBuffer)
{
	register unsigned long  localIdentSize = 0;
	while(dChildIdentBuffer[localIdentSize] != ':' &&
			dChildIdentBuffer[localIdentSize] != '\0')
		++localIdentSize;

	Namespace *nextInPath = searchOnly(dChildIdentBuffer, localIdentSize);

	if(dChildIdentBuffer[localIdentSize] == '\0')
		return (nextInPath);
	else
		Dbg(dChildIdentBuffer + localIdentSize);

	return ((nextInPath != null) ? nextInPath->searchRelative(
			dChildIdentBuffer + localIdentSize + 2) : null);
}

Namespace *Namespace::search(const char *globalIdentifier, bool object)
{
	return (Namespace::root()->searchRelative(globalIdentifier + 2));
}
