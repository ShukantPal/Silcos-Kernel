/*=++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * File: RSDT.h
 *
 * Summary: This file contains the interface for finding and using the RSDT.
 *
 * Functions:
 * SetupRsdt() - This function is used for mapping and initializating the ACPI subsystem.
 * RetrieveConfiguration() - This function is used for getting a specific ACPI table.
 *
 *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=*/
#ifndef CONFIG_RSDT_H
#define CONFIG_RSDT_H

#include "SDTHeader.h"

/******************************************************************************
 * Type: RSDT
 * 
 * Summary: This type represents the RSDT (from ACPI), which contains references to other
 * ACPI tables.
 *
 * Fields:
 * SDT_HEADER RootHeader - SDT_HEADER of the RSDT
 * U32 ConfigurationTables - Physical addresses (32-bit) to ACPI tables (other than RSDT)
 *
 * @Version 1
 * @Since Circuit 2.03
 ******************************************************************************/
typedef
struct {
	SDT_HEADER RootHeader;
	U32 ConfigurationTables[];
} RSDT;

extern RSDT *SystemRsdt;/* RSDT found during setup */

/******************************************************************************
 * Function: SetupRsdt() 
 *
 * Summary: After finding the RSDP, the kernel calls this function to map the RSDT, and also
 * other ACPI tables.
 *
 * @Version 1
 * @Since Circuit 2.03
 ******************************************************************************/
VOID SetupRsdt(
	VOID
);

/******************************************************************************
 * Function: RetrieveConfiguration()
 *
 * Summary: This function is used to search for a particular ACPI table.
 *
 * Fields:
 * RootSDT - RSDT used
 * Signature - String-identifier for the table
 * MappingBlk - Kernel-space for ACPI tables block
 *
 * @Version 1
 * @Since Circuit 2.03
 ******************************************************************************/
VOID *RetrieveConfiguration(
	RSDT *RootSDT,
	CHAR *Signature,
	U32 MappingBlk
);

#endif /* Config/RSDT.h */
