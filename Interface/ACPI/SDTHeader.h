/**
 * Copyright (C) 2017 - Shukant Pal
 */

#ifndef CONFIG_SDT_HEADER_H
#define CONFIG_SDT_HEADER_H

#define RsdtSignature "RSDT"
#define MadtSignature "APIC"
#define FadtSignature "FACP"

#include <TYPE.h>

/**
 * Type: SDT_HEADER
 *
 * Summary: This type is used for unifing the ACPI tables and giving them a common header.
 *
 * Fields:
 * S8 Signature[4] - This contains the specific ACPI table string-identifier
 * U32 Length - Size of the table
 * U8 Revision - Version of the table
 * U8 Checksum -
 * S8 OemId[6]
 */
typedef
struct _SDT_HEADER
{
	S8 Signature[4];
	U32 Length;
	U8 Revision;
	U8 Checksum;
	S8 OemId[6];
	S8 OemTableId[8];
	U32 OemRevision;
	U32 CreatorId;
	U32 CreatorRevision;
} SDT_HEADER;

static inline
BOOL VerifySdtChecksum(SDT_HEADER *Sdt) {
	register U8 Sum = 0;

	for(register ULONG Counter = 0; Counter < Sdt -> Length; Counter++)
		Sum += ((U8*) Sdt)[Counter];

	return (Sum == 0);
}

#endif /* Config/Sdt.h */
