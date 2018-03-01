///
/// @file ElfAnalyer.hpp
///
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

#ifndef INTERFACE_MODULE_ELFANALYZER_HPP_
#define INTERFACE_MODULE_ELFANALYZER_HPP_

#include "ELF.h"
#include <KERNEL.h>

namespace Module
{
namespace Elf
{

class ElfManager;

///
/// @class ElfAnalyzer
///
/// Contains static functions which are used for analyzing elf-objects and
/// performing various operations related to them.
///
/// @version 1.2
/// @since Silcos 2.05
/// @author Shukant Pal
///
class ElfAnalyzer
{
public:
	static bool validateBinary(Void *fileBinary) kxhide;
	static ElfManager *getSecureManager() kxhide;
	static unsigned long getSymbolHash(char *symbolName) kxhide;

	//! not implemented yet
	static unsigned long getGNUSymbolHash(char *symbolName) kxhide;
	static Symbol* querySymbol(char *name, SymbolTable *symbolTable,
					HashTable *hashTable) kxhide;
//	static Symbol* querySymbol(char *name, SymbolTable *symbolTable,
//					GNUHashTable *gnuHash) kxhide;
};
}
}


#endif /* INTERFACE_MODULE_ELFANALYZER_HPP_ */
