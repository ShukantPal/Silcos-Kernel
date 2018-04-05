///
/// @file ModuleContainer.cpp
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

#include <Module/ModuleContainer.hpp>

using namespace Module;

SymbolLookup *defPtpGroup;

///
/// Formal constructor for the ModuleContainer, although most fields are to
/// be filled later. It holds the build-time name & version, symbol pool,
/// and initial Namespace object information  only.
///
/// @param buildName - build-time name of the module
/// @param buildVersion - version of the kernel module build
/// @version 1.0
/// @since Silcos 3.05
/// @author Shukant Pal
///
ModuleContainer::ModuleContainer(const char *buildName,
		unsigned long buildVersion)
: Namespace((const char *) buildName, Namespace::root()),
  ptpResolvableLinks(defPtpGroup)
{
	this->buildName = static_cast<const char*>(kmalloc(strlen(buildName)));
	strcpy(buildName, const_cast<char*>(this->buildName));

	this->buildVersion = buildVersion;
	this->physicalAddress = this->baseAddress = 0;
	this->initFunctor = this->kMain = this->finiFunctor =
			static_cast<void (*)()>(null);
	this->linkerInfo = static_cast<DynamicLink *>(null);
	addToParent();
}
