/**
 * File: MADT.h
 *
 * Summary:
 * Defines the MADT (Multiple APIC Descriptor Table) driver which can parse and
 * interpret contained information.
 *
 * Functions:
 * EnumerateMADT() - Used for serially invoking handler functions for each MADT-entry.
 *
 * Copyright (C) 2017 - Shukant Pal
 */
#ifndef ACPI_MADT_H__
#define ACPI_MADT_H__

#include "SDTHeader.h"

struct MADT
{
	SDT_HEADER baseHeader;
	U32 LCA;
	U32 flags;
	U8 SystemInfo[ ];// Array of various entries, starting with a MADTHeader
};

struct MADTHeader
{
	U8 entryType;
	U8 recordLength;
};

struct MADTEntryLAPIC
{
	U8 acpiID;
	U8 apicID;
	U32 configFlags;
};

struct MADTEntryIOAPIC
{
	U8 apicID;
	U8 reserved;
	U32 apicBase;
	U32 GSIB;
};

struct MADTEntryISR
{
	U8 busSource;
	U8 irqSource;
	U32 GSI;
	U16 configFlags;
};

void EnumerateMADT(void (*handleAPIC)(MADTEntryLAPIC*),
			void (*handleIOAPIC) (MADTEntryIOAPIC*),
			void (*handleISR)(MADTEntryISR*));
#endif/* ACPI/MADT.h */
