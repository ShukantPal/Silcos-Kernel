/**
 * File: RecordManager.cpp
 *
 * Summary:
 * This file implements the module-record organization provided. Global-level
 * information querying and overall record I/O is given in the RecordManager
 * implementation.
 * 
 * Functions:
 * RecordManager::createRecord - Create a new record, without registration
 * RecordManager::querySymbol - Search for a symbol by name, in all records
 *
 * Origin:
 *
 * Copyright (C) 2017 - Shukant Pal
 */
#include <Module/Elf/ElfAnalyzer.hpp>
#include <Module/ModuleRecord.h>
#include <Util/Memory.h>
#include <KERNEL.h>

using namespace Module;

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
 * CHAR *modName - Name of the registering module
 * ULONG buildVersion - Version for the module build
 * ULONG serviceType - Type of service provided by the module (@See ModuleType)
 *
 * Origin:
 * This creates a valid-record for the RecordManager.
 *
 * Version: 1.1
 * Since: Circuit 2.03++
 * Author: Shukant Pal
 */
ModuleRecord *RecordManager::createRecord(
		CHAR *modName,
		ULONG buildVersion,
		ULONG serviceType
){
	ModuleRecord *newRecord = new(tKMOD_RECORD) ModuleRecord();

	memcpy(modName, &newRecord->buildName, 16);
	newRecord->buildVersion = buildVersion;
	newRecord->serviceType = serviceType;

	return (newRecord);
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
 * CHAR *requiredSymbolName - Name of the symbol being queried
 *
 * Version: 1.0
 * Since: Circuit 2.03
 * Author: Shukant Pal
 */
Symbol *RecordManager::querySymbol(
		CHAR *requiredSymbolName
){
	ModuleRecord *qRecord = (ModuleRecord*) RecordManager::globalRecordList.Head;
	ULONG qRecordIndex = 0;
	ULONG qRecordCount = RecordManager::globalRecordList.Count;

	DynamicLink *qLink;
	Symbol *foundSymbol;

	while(qRecordIndex < qRecordCount){
		qLink = qRecord->linkerInfo;
		foundSymbol = ElfAnalyzer::querySymbol(requiredSymbolName, &qLink->dynamicSymbols, &qLink->symbolHash);

		if(foundSymbol != NULL){
			if(foundSymbol->Value != 0)
				return (foundSymbol);
		}

		++(qRecordIndex);
		++(qRecord);
	}

	return (NULL);
}
