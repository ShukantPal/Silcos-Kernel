/**
 * File: RSDT.h
 *
 * Summary:
 * Declares the driver to find & parse the ACPI-RSDT.
 *
 * Functions:
 * SetupRsdt() - search & store the RSDT (at boot)
 * RetrieveConfiguration() - parse RSDT & get another ACPI table
 *
 * Copyright (C) 2017 - Shukant Pal
 */
#ifndef CONFIG_RSDT_H
#define CONFIG_RSDT_H

#include "SDTHeader.h"

/**
 * Type: RSDT
 * 
 * Summary:
 * Defines the structure of the root-system-descriptor-table which is filled
 * by the ACPI to allow (this kernel) to access hardware configurations.
 *
 * Author: Shukant Pal
 */
struct RSDT
{
	SDTHeader RootHeader;// Standard ACPI Header
	U32 ConfigurationTables[];// Physical-addresses of ACPI Tables

	inline unsigned long entryCount()
	{
		return (RootHeader.Length - sizeof(SDTHeader)) / 4;
	}
};

/*
 * Just like RSDT, but is created by the kernel. It actually maps each ACPI
 * table to "virtual" addresses that are directly usable.
 */
struct VirtualRSDT
{
	RSDT *physTable;
	unsigned long matrixBase;
	unsigned long stdTableCount;
	unsigned long stdTableAddr[];
};

extern RSDT *SystemRsdt;/* RSDT found during setup */

void SetupRSDTHolder(void);
void *SearchACPITableByName(const char *tblSign, SDTHeader *stdTable);

#endif/* ACPI/RSDT.h */
