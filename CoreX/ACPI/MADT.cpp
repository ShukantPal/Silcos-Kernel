/**
 * Copyright (C) 2017 - Shukant Pal
 */

#include <ACPI/SDTHeader.h>
#include <ACPI/RSDT.h>
#include <ACPI/MADT.h>
#include <Memory/KMemorySpace.h>

VOID EnumerateMADT(VOID (*HandleLAPIC) (MADT_ENTRY_LAPIC *), VOID (*HandleIOAPIC) (MADT_ENTRY_IOAPIC *), VOID (*HandleISR) (MADT_ENTRY_ISR *)){
	MADT *madtPtr = RetrieveConfiguration(SystemRsdt, MadtSignature, ConfigBlock);
	for(ULONG tableOffset=0; tableOffset<(madtPtr->BaseHeader.Length - sizeof(MADT) - 8);) {
		switch(madtPtr->SystemInfo[tableOffset]) {
			case 0:
				if(HandleLAPIC != NULL)
					HandleLAPIC((MADT_ENTRY_LAPIC*) &madtPtr->SystemInfo[tableOffset + 2]);
				break;
			case 1:
				if(HandleIOAPIC != NULL)
					HandleIOAPIC((MADT_ENTRY_IOAPIC*) &madtPtr->SystemInfo[tableOffset + 2]);
				break;
			case 2:
				if(HandleISR != NULL)
					HandleISR((MADT_ENTRY_ISR*) &madtPtr->SystemInfo[tableOffset + 2]);
				break;
			default:
				return;
		};

		tableOffset += madtPtr->SystemInfo[tableOffset + 1];
	}
}
