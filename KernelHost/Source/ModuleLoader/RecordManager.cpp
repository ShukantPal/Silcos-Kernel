///
/// @file RecordManager.cpp
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
#include <Module/Elf/ElfAnalyzer.hpp>
#include <Module/ModuleRecord.h>
#include <Utils/Memory.h>
#include <KERNEL.h>

using namespace Module;
using namespace Module::Elf;

/*
 * This list contains all the registered module-records, unbiased of any
 * ABI filters.
 */
LinkedList RecordManager::globalRecordList;

/**
 * Function: RecordManager::createRecord
 *
 * Summary:
 * This function is sort of a 'constructor' or a factory method for the ModuleRecord
 * type.
 *
 * Args:
 * char *modName - Name of the registering module
 * unsigned long buildVersion - Version for the module build
 * unsigned long serviceType - Type of service provided by the module (@See ModuleType)
 *
 * Origin:
 * This creates a valid-record for the RecordManager.
 *
 * Version: 1.1
 * Since: Circuit 2.03++
 * Author: Shukant Pal
 */
ModuleRecord *RecordManager::create(char *modName, unsigned long buildVersion,
						unsigned long serviceType)
{
	ModuleRecord *newRecord = new(tKMOD_RECORD) ModuleRecord();

	memcpy(modName, &newRecord->buildName, 16);
	newRecord->buildVersion = buildVersion;
	newRecord->serviceType = (KM_TYPE) serviceType;

	return (newRecord);
}

const ModuleRecord *RecordManager::search(char *name)
{
	ModuleRecord *rec = (ModuleRecord*) globalRecordList.head;

	while(rec != NULL)
	{
		if(strcmpn(rec->buildName, name, 16))
			return ((const ModuleRecord*) rec);

		rec = rec->NextModule;
	}

	return (null);
}

/**
 * Function: RecordManager::querySymbol
 *
 * Summary:
 * This function is used for querying a symbol by its name at the global level,
 * by going through each ModuleRecord. It provides a ABI-independent interface
 * for this, and no filters are applied.
 *
 * Args:
 * char *requiredSymbolName - Name of the symbol being queried
 * unsigned long& baseAddress - (Return) arg for giving base-address of declarer
 *
 * Version: 1.0
 * Since: Circuit 2.03
 * Author: Shukant Pal
 */
Symbol *RecordManager::querySymbol(char *requiredSymbolName, unsigned long &baseAddress)
{
	ModuleRecord *qRecord = (ModuleRecord*) RecordManager::globalRecordList.head;
	unsigned long qRecordIndex = 0;
	unsigned long qRecordCount = RecordManager::globalRecordList.count;

	DynamicLink *qLink;
	Symbol *foundSymbol;

	while(qRecordIndex < qRecordCount)
	{
		qLink = qRecord->linkerInfo;
		foundSymbol = ElfAnalyzer::querySymbol(requiredSymbolName, &qLink->dynamicSymbols, &qLink->symbolHash);

		if(foundSymbol != NULL)
		{
			if(foundSymbol->Value != 0)
			{
				baseAddress = qRecord->BaseAddr;
				return (foundSymbol);
			}
		}

		++(qRecordIndex);
		qRecord = qRecord->NextModule;
	}

	return (NULL);
}
