///
/// @file SymbolLookup.hpp
///
/// Provides utility object to directly manipulate symbolic definitions
/// at a low-level in all types of kernel binary interfaces. This allows
/// modules to auto-create symbols and allows dynamic-code generation in
/// the case of a in-kernel JIT compiler.
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

#ifndef KERNHOST_MODULE_SYMBOLLOOKUP_HPP_
#define KERNHOST_MODULE_SYMBOLLOOKUP_HPP_

namespace Module
{

class ModuleContainer;

enum SymbolType
{
	DATA,
	FUNC,
	NOTE,
	FILE,
	OTHER
};

///
/// Holds all the symbolic definitions that have been given kernel-modules
/// in one internal hash-table to allow faster-lookups. It uses the GNU hash
/// algorithm to generate hash-keys for symbol-names.
///
/// It is used to store all kernel-symbols in the environment and also for
/// looking up **local** symbols in a module.
///
/// This lookup-class is able to hold 16,384 symbol buckets.
///
/// @version 1.0
/// @since Silcos 3.02
/// @author Shukant Pal
///
class SymbolLookup final
{
public:
	SymbolLookup();
	~SymbolLookup();
	void add(Elf::Symbol& esym, char *nameTable, ModuleContainer *mcont);
	void addAll(Elf::SymbolTable &table, ModuleContainer *mcont);
	unsigned long lookup(char *symbolName, ModuleContainer* &owner);
	//void removeAll(Elf::SymbolTable &table);
private:
	struct SymbolicDefinition
	{
		SymbolicDefinition *next;
		SymbolicDefinition *last;
		ModuleContainer *sandBox;
		unsigned long address;
		SymbolType type;
		char name[0];
	};

	unsigned long internalCapacity;
	unsigned long currentThreshold;
	unsigned long totalSymbols;
	SymbolicDefinition **lookupBuckets;
	ReadWriteSerializer rwl;
	bool addDirect(unsigned long address, SymbolType type,
			unsigned char *name, ModuleContainer *mcont);
	void ensureCapacity(unsigned long newSize);
	void move(SymbolicDefinition *symdef, SymbolicDefinition **oldBucket,
			SymbolicDefinition **newBucket);
	void moveAll(SymbolicDefinition **newBuckets,
			unsigned long newCapacity);
	unsigned long hashKey(unsigned char *name);
	unsigned long hashKey(unsigned char *name, unsigned long& count);
#define indexFor(sdef) \
	(hashKey((unsigned char*)(sdef).name) % internalCapacity)
#define newIndexFor(anySym, newCapacity) \
	(hashKey((unsigned char *)(anySym->name)) % newCapacity)
};

extern SymbolLookup *globalKernelSymbolTable;//!< This is used for

}

#endif/* Module/SymbolLookup.hpp */
