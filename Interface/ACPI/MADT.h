/*=++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * File: MADT.h
 *
 * Summary: This file contains the interface to use and retrieve information from the APIC MADT.
 *
 * Functions:
 * EnumerateMADT() - Used for serially invoking handler functions for each MADT-entry.
 *
 * Copyright (C) 2017 - Shukant Pal
 *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=*/
#ifndef ACPI_MADT_H
#define ACPI_MADT_H

#include "SDTHeader.h"

/******************************************************************************
 * Type: MADT
 *
 * Summary: This type represent the 'MADT' table in the ACPI standard, and contains the
 * system information on APICs, IOAPICs, and ISRs present.
 *
 * Fields:
 * BaseHeader - This field contains the SDT_HEADER
 * LCA -
 * Flags - 
 * SystemInfo - Array of various entries, each starting with a MADT_HEADER
 *
 * @Version 1
 * @Since Circuit 2.03 
 ******************************************************************************/
typedef
struct _MADT {
	SDT_HEADER BaseHeader;
	U32 LCA;
	U32 Flags;
	U8 SystemInfo[ ];
} MADT;

/******************************************************************************
 * Type: MADT_HEADER
 *
 * Summary: This type is the starting of each MADT entry, and contains its type and length.
 *
 * Fields:
 * U8 EntryType - This contains the type of the entry which can be of the following value ->
 * 1. MADT_ENTRY_TYPE_LAPIC, 
 * 2. MADT_ENTRY_TYPE_IOAPIC, or 
 * 3. MADT_ENTRY_TYPE_ISR
 *
 * U8 RecordLength - Total size of the entry
 *
 * @Version 1
 * @Since Circuit 2.03
 ******************************************************************************/
typedef
struct _MADT_HEADER {
	U8 EntryType;
	U8 RecordLength;
} MADT_HEADER;

/******************************************************************************
 * Type: MADT_ENTRY_LAPIC
 * 
 * Summary: This type represents a APIC entry in the MADT.
 *
 * Fields:
 * U8 ACPIID - ACPI ID provided by the system
 * U8 APICID - APIC ID given during BIOS initialization
 * U32 Flags - 
 *
 * @Version 1
 * @Since Circuit 2.03
 ******************************************************************************/
typedef
struct _MADT_ENTRY_LAPIC {
	U8 ACPIID;
	U8 APICID;
	U32 Flags;
} MADT_ENTRY_LAPIC;

/******************************************************************************
 * Type: MADT_ENTRY_IOAPIC
 *
 * Summary: This type represents a IOAPIC entry in the MADT.
 *
 * Fields:
 * U8 APICID - APIC ID given during BIOS initialization
 * U8 Reserved - Unrefered field
 * U32 IOAPICAddress - Physical address of the IOAPIC registers
 * U32 GSIB - 
 *
 * @Version 1
 * @Since Circuit 2.03
 ******************************************************************************/
typedef
struct _MADT_ENTRY_IOAPIC {
	U8 APICID;
	U8 Reserved;
	U32 IOAPICAddress;
	U32 GSIB;
} MADT_ENTRY_IOAPIC;

/******************************************************************************
 * Type: MADT_ENTRY_ISR
 *
 * Summary:
 *
 * Fields:
 * U8 BusSource -
 * U8 IRQSource -
 * U32 GSI -
 * U16 Flags -
 *
 * @Version 1
 * @Since Circuit 2.03
 ******************************************************************************/
typedef
struct _MADT_ENTRY_ISR {
	U8 BusSource;
	U8 IRQSource;
	U32 GSI;
	U16 Flags;
} MADT_ENTRY_ISR;

/******************************************************************************
 * Function: EnumerateMADT()
 *
 * Summary: This function is used to serially go through the MADT, and invoke handler
 * function for each entry-type.
 *
 * Args:
 * VOID (*handleLAPIC) (MADT_ENTRY_LAPIC *) - Handle LAPIC entry
 * VOID (*handleIOAPIC) (MADT_ENTRY_IOAPIC *) - Handle IOAPIC entry
 * VOID (*handleISR) (MADT_ENTRY_ISR *) - Handle ISR entry
 *
 * @Version 1
 * @Since Circuit 2.03
 ******************************************************************************/
VOID EnumerateMADT(
	VOID (*handleLAPIC) (MADT_ENTRY_LAPIC *),
	VOID (*handleIOAPIC) (MADT_ENTRY_IOAPIC *),
	VOID (*handleISR) (MADT_ENTRY_ISR *)
);

#endif /* ACPI/MADT.h */
