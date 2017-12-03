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
struct RSDT {
	SDT_HEADER RootHeader;// Standard ACPI Header
	U32 ConfigurationTables[];// Physical-addresses of ACPI Tables
};

extern RSDT *SystemRsdt;/* RSDT found during setup */

void SetupRsdt(void);
void *RetrieveConfiguration(RSDT *RootSDT, const char *Signature,
				U32 MappingBlk);

#endif/* ACPI/RSDT.h */
