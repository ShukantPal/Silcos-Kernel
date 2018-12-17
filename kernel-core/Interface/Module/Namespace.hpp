///
/// @file Namespace.hpp
///
/// Features support for binary-namespaces under which modules can be
/// securely linked to a set of known modules.
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

#ifndef NAMESPACE_HPP_
#define NAMESPACE_HPP_

#include <Object.hpp>
#include <Utils/LinkedList.h>

namespace Module
{

///
/// Encapsulates multiple-functionalities that can be used to manipulate
/// binary-file objects and kernel modules. It is a part of a huge tree that
/// maintains holds topological information on kernel modules and can be
/// used to restrict linking and execution of a module to a specific domain
/// of modules.
///
/// For example, a IPC-extension module declares a class CommChannel and
/// the UDI environment internally also uses a class CommChannel. The symbols
/// won't clash because the IPC-extension will remain in the KAF sub-namespace
/// while UDI stuff will be kept in the Driver namespace.
///
///
class Namespace: protected LinkedListNode
{
public:
	const char *fileName;
	const char *nsLocalIdentifier;
	const char *nsGlobalIdentifier;

	Namespace *create(const char *name)
	{
		if (searchOnly(name, strlen(name)) == null) {
			Namespace *newChild = new Namespace(name, this);
			AddElement(newChild, &nsChildSet);
			return (newChild);
		} else {
			return (null);
		}
	}

	Namespace *create(const char *fromFile, const char *name)
	{
		if (searchOnly(name, strlen(name)) == null) {
			Namespace *newChild = new Namespace(fromFile, name, this);
			AddElement(newChild, &nsChildSet);
			return (newChild);
		} else {
			return (null);
		}
	}

	Namespace *searchOnly(const char *dChildIdentBuffer, unsigned bytes);
	Namespace *searchRelative(const char *dChildIdentBuffer);

	static void ensureInit();
	static Namespace *search(const char *globalIdentifier, bool object);
	static Namespace *root()
	{
		return (const_cast<Namespace*>(&nsRoot));
	}
protected:
	Namespace *nsOwner;
	LinkedList nsChildSet;
	unsigned long totalChildren;

	Namespace(const char *name, Namespace *owner);
	Namespace(const char *fileName, const char *name, Namespace *owner);

	void addToParent()
	{
		AddElement(this, &nsOwner->nsChildSet);
	}
private:
	static Namespace nsRoot;
	Namespace();
};

} // namespace Module

#endif/* Module/Namespace.hpp */
