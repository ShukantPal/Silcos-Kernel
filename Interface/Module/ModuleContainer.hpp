///
/// @file ModuleContainer.hpp
///
/// Provides high-level abstraction of modules, module-segments, symbol
/// definitions, symbol references, weak-symbols;
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

#ifndef KERNHOST_MODULE_MODULECONTAINER_HPP__
#define KERNHOST_MODULE_MODULECONTAINER_HPP__

#include "Elf/ELF.h"
#include "ModuleRecord.h"
#include "Namespace.hpp"
#include "SymbolLookup.hpp"

namespace Module
{

class SymbolLookup;

///
/// Represents a module-file which an binary-interface independent
/// manner. 
///    
///
/// @version 1.0
/// @since Silcos 3.02
/// @author Shukant Pal
///
class ModuleContainer : protected Namespace
{
public:
	//!
	//! Holds the peer-to-peer symbolic definitions that can be used for
	//! linking with fellow sibling modules. It is generally provided by
	//! parent-module. A few exceptions exist like the in-built modules.
	//!
	//! It also holds definitions of parent module, if it exists.
	//!
	SymbolLookup *ptpResolvableLinks;
	
	//!
	//! Holds the SymbolLookup object that is used by direct children of
	//! this module. It holds symbolic definitions of direct-children, if
	//! any. If the parent-namespace is a ModuleContainer, then it will
	//! automatically dump its definitions in this buffer so that its
	//! children can use them.
	//!
	const SymbolLookup isoResolvableBuffer;

	void (*initFunctor)(), (*kMain)(), (*finiFunctor)();
	void (**preInitArray)(), (**initArray)(), (**finiArray)();
	size_t preInitFunctorCount, initFunctorCount, finiFunctorCount;

	unsigned long getBase(){ return (baseAddress); }
	DynamicLink *getLinker(){ return (linkerInfo); }
	const char *ident(){ return (buildName); }
	void init(){ initFunctor(); }
	void fini(){ finiFunctor(); }
	unsigned long load(){ return (physicalAddress); }
	void setBase(unsigned long base){ baseAddress = base; }
	void setLinker(DynamicLink *nli){ linkerInfo = nli; }

	ModuleContainer(const char *buildName, unsigned long buildVersion);
protected:
	~ModuleContainer();
private:
	const char *buildName;
	unsigned long buildVersion;
	DynamicLink *linkerInfo;
	VirtAddr baseAddress;
	PhysAddr physicalAddress;
};

} // namespace Module

extern Module::SymbolLookup *defPtpGroup;

#endif/* Module/ModuleContainer.hpp */
