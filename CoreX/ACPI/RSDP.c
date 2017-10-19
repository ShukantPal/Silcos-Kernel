/**
 * Copyright (C) 2017 - Shukant Pal
 */

#include <ACPI/RSDP.h>
#include <Memory/Pager.h>
#include <Util/Memory.h>
#include <KERNEL.h>

RSDP *SystemRsdp = NULL;
RSDP2 *SystemRsdp2 = NULL;

RSDP *ScanRsdp(){
	UCHAR *ScanAddress = (UCHAR *) 0xc0000000;
	CHAR *RsdpString = RsdpSignature;

	DbgLine("Scanning Configuration...");

	while((U32) ScanAddress < RsdpThreshold) {
		if(memcmp(ScanAddress, RsdpString, 8)) {
			if(VerifyRsdpChecksum((RSDP *) ScanAddress)) {
				SystemRsdp = (RSDP *) ScanAddress;
				break;
			}
		}

		ScanAddress += 16;
	}

	if(!SystemRsdp) {
		DbgLine("System Hardware Failure : ");
		DbgLine("Details : Root System Description not found. ");
		DbgLine("Warning - Severe.");
		asm volatile("hlt;");
	}

	return (SystemRsdp);
}
