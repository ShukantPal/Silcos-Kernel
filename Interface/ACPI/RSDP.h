/*=++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * File: RSDP.h
 *
 * Summary: This file contains the interface to detect and verify the RSDP provided by the system, and
 * also the reach the RSDT.
 *
 * Functions:
 * VerifyRsdpChecksum() - Used to verify the checksum of a RSDP
 *
 * Copyright (C) 2017 - Shukant Pal
 *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=*/
#ifndef ACPI_RSDP_H
#define ACPI_RSDP_H

#define RsdpSignature "RSD PTR "
#define RsdpThreshold (0xc0000000 + 1024 * 1024)

#include <TYPE.h>

/******************************************************************************
 * Type: RSDP
 *
 * Summary: This type represents the firmware-provided RSDT pointer.
 *
 * Fields:
 * S8 Signature[8] - This contains the 'RsdpSignature' string.
 * U8 Checksum - This plus all other fields must equal 0
 * S8 OemId[6] - Contains the OEM identifier
 * Revision - Version no. of the ACPI in the system
 * RsdtAddress - Physical address of the RSDT
 *
 * @Version 1
 * @Since Circuit 2.03
 ******************************************************************************/
typedef
struct {
	S8 Signature[8];
	U8 Checksum;
	S8 OemId[6];
	U8 Revision;
	U32 RsdtAddress;
} __attribute__((packed)) RSDP;

/******************************************************************************
 * Type: RSDP2
 *
 * Summary: This type represents the new version of the RSDP.
 *
 * Fields:
 * RSDP BaseDescriptor - RSDP header (first version/backward compatible)
 * U32 Length - Size of the header
 * U64 XsdtAddress - XSDT address (64-bits)
 * U8 ExtendedChecksum - This plus all other fields must equal 0
 * U8 Reserved[3] - Unreferred field
 *
 * @Version 1
 * @Since Circuit 2.03
 ******************************************************************************/
typedef
struct {
	RSDP BaseDescriptor;
	U32 Length;
	U64 XsdtAddress;
	U8 ExtendedChecksum;
	U8 Reserved[3];
} __attribute__((packed)) RSDP2;

/******************************************************************************
 * Function: ScanRsdp()
 *
 * Summary: This function scans for the RSDP in the EDBA and all, and returns it.
 *
 * Returns: RSDP* if find
 *
 * @Version 1
 * @Since Circuit 2.03 
 ******************************************************************************/
RSDP *ScanRsdp(
	VOID
);

extern RSDP *SystemRsdp;/* RSDP found during setup */
extern RSDP2 *SystemRsdp2;/* RSDP2 found during setup (if x64 architecture) */

/******************************************************************************
 * Function: VerifyRsdpChecksum()
 *
 * Summary: This function verifies the RSDP by checking that its checksum is correct.
 *
 * Args:
 * Rsdp - RSDP structure provided
 *
 * @Version 1
 * @Since Circuit 2.03
 ******************************************************************************/
static inline
BOOL VerifyRsdpChecksum(RSDP *Rsdp) {
	U8 Sum = 0;

	for(ULONG Counter = 0; Counter < sizeof(RSDP); Counter++) {
		Sum += ((U8*) Rsdp)[Counter];
	}

	return (Sum == 0);
}

#endif /* ACPI/RSDP.h */
