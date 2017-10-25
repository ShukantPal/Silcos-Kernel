/**
 * Copyright (C) 2017 - Shukant Pal
 */

#include <ACPI/RSDP.h>
#include <ACPI/RSDT.h>
#include <ACPI/XSDT.h>
#include <Memory/Pager.h>
#include <Memory/KMemorySpace.h>
#include <Util/Memory.h>
#include <KERNEL.h>

RSDT *SystemRsdt;
XSDT *SystemXsdt;

CHAR *sysRSDTNotFound = "RSDT NOT FOUND";

VOID SetupRsdt(){
	if(SystemRsdp->Revision == 0) {
		EnsureMapping(ConfigBlock * 4096, SystemRsdp -> RsdtAddress & 0xfffff000, NULL, KF_NOINTR, 3);

		RSDT *Rsdt = (RSDT *) (ConfigBlock * 4096 + (SystemRsdp -> RsdtAddress % 4096));
		SystemRsdt = Rsdt;
		++ConfigBlock;

		if(!VerifySdtChecksum(&(SystemRsdt -> RootHeader))) {
			DbgLine(sysRSDTNotFound);
			asm volatile("cli; hlt;");
		}
	}
}

VOID *RetrieveConfiguration(RSDT *RootSDT, CHAR *Signature, U32 MappingBlk){
	U32 ConfigEntries = (SystemRsdt -> RootHeader.Length - sizeof(SDT_HEADER)) / 4;
	SDT_HEADER *Sdt = NULL;  

    	for (U32 Index = 0; Index < ConfigEntries; Index++) {
		EnsureMapping(MappingBlk * 4096, SystemRsdt -> ConfigurationTables[Index] & 0xfffff000, NULL, KF_NOINTR, KernelData);
		Sdt = (SDT_HEADER *) (MappingBlk* 4096 + SystemRsdt -> ConfigurationTables[Index] % 4096);
		if(memcmp(Sdt, Signature, 4))
			return (VOID *) (Sdt);
	}
 
	return (NULL);
}
