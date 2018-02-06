/* @file MADT.cpp
 *
 * Copyright (C) 2017 - Shukant Pal
 */
#include <ACPI/SDTHeader.h>
#include <ACPI/RSDT.h>
#include <ACPI/MADT.h>
#include <Memory/KMemorySpace.h>

/*
 * Enumerates the multiple-descriptor APIC table (ACPI 2.0) and calls the
 * handlers for each type of entry. Currently, nominal entries local & IO
 * APIC and Interrupt Service Routine entries are supported here.
 *
 * @version 1.0
 * @since Silcos 3.02
 * @author Shukant Pal
 */
void EnumerateMADT(void (*handleLAPIC)(MADTEntryLAPIC *),
				void (*handleIOAPIC)(MADTEntryIOAPIC *),
				void (*handleISR)(MADTEntryISR *))
{
	MADT *madtPtr = (MADT*) SearchACPITableByName("APIC", null);
	U8 *entryBytes = &madtPtr->SystemInfo[0];
	unsigned long jump;

	for(unsigned long tableOffset = 0;
			tableOffset<(madtPtr->baseHeader.Length-sizeof(MADT)-8)
			;)
	{
		switch(*entryBytes)
		{
		case 0:
			if(handleLAPIC != NULL)
				handleLAPIC((MADTEntryLAPIC*)(entryBytes + 2));
			break;
		case 1:
			if(handleIOAPIC != NULL)
				handleIOAPIC((MADTEntryIOAPIC*)(entryBytes + 2));
			break;
		case 2:
			if(handleISR != NULL)
				handleISR((MADTEntryISR*)(entryBytes + 2));
			break;
		default:
			return;
		};

		jump = *(entryBytes + 1);
		tableOffset += jump;
		entryBytes += jump;
	}
}
