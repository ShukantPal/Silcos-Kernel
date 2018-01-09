/**
 * Copyright (C) 2017 - Shukant Pal
 */
#include <ACPI/RSDP.h>
#include <Memory/Pager.h>
#include <Util/Memory.h>
#include <KERNEL.h>

RSDP *SystemRsdp = NULL;
RSDP2 *SystemRsdp2 = NULL;

RSDP *ScanRsdp()
{
	unsigned char *scanAddress = (unsigned char *) 0xc0000000;
	char *RsdpString = RsdpSignature;

	DbgLine("Scanning Configuration...");

	while((U32) scanAddress < RsdpThreshold)
	{
		if(memcmp(scanAddress, RsdpString, 8))
		{
			if(VerifyRsdpChecksum((RSDP *) scanAddress))
			{
				SystemRsdp = (RSDP *) scanAddress;
				break;
			}
		}

		scanAddress += 16;
	}

	if(!SystemRsdp)
	{
		DbgLine("System Hardware Failure : ");
		DbgLine("Details : Root System Description not found. ");
		DbgLine("Warning - Severe.");
		asm volatile("hlt;");
	}

	return (SystemRsdp);
}
