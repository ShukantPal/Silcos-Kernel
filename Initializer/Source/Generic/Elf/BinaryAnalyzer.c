/**
 * @file BinaryAnalyzer.c
 *
 *  Created on: 13-Jun-2018
 *      Author: sukantdev
 */
#define _CBUILD
#include <ArchGeneric/MemConfig.h>
#include <Generic/Elf/BinaryAnalyzer.h>
#include <Generic/EarlyConsole.h>

/**
 * Validates the ELF-file by checking its magic-bytes, after casting the file
 * pointer into a ELF-header.
 *
 * @param file - the pointer to the file being checked, in memory
 * @return - whether the given file is an ELF object.
 */
bool IsElfFile(void *file)
{
	struct ElfHeader *fileAlias = (struct ElfHeader *) file;

	if(fileAlias->fileIdentifier[EI_MAG0] != ELFMAG0) return (false);
	if(fileAlias->fileIdentifier[EI_MAG1] != ELFMAG1) return (false);
	if(fileAlias->fileIdentifier[EI_MAG2] != ELFMAG2) return (false);
	if(fileAlias->fileIdentifier[EI_MAG3] != ELFMAG3) return (false);

	return (true);
}

struct DynamicEntry *FindEntryByTag(struct DynamicEntry *dynEnt,
		unsigned long tagReq)
{
	while(dynEnt->tag != 0) {
		if(dynEnt->tag == tagReq)
			 return (dynEnt);
		++(dynEnt);
	}

	return (null);
}

/**
 * Scans a elf-object for its dynamic symbols. It returns the number of
 * symbols found and the size of the string-table used for identifying them.
 *
 * Note that the count of symbols and size of the string-table, are added
 * to the output arguments given.
 *
 * @param[in] fileHdr - the elf-header of the object being parsed
 * @param[out] symbolCount - the count of the symbols in the object
 * @param[out] stringSize - the size of the string table
 */
void ScanSymbols(struct ElfHeader *fileHdr, unsigned long *symbolCount,
		unsigned long *stringSize)
{
	struct ProgramHeader *segmentEntry = (struct ProgramHeader *)
			((unsigned long) fileHdr + fileHdr->programHeaderOffset);
	unsigned long segEntryCount = fileHdr->programHeaderEntryCount;

	for(unsigned long index = 0;
			index < segEntryCount;
			index++, segmentEntry++) {
		if(segmentEntry->entryType == PT_DYNAMIC)
			break;
	}

	if(segmentEntry->entryType != PT_DYNAMIC) {
		WriteLine("Warning: Invalid ELF-object found: No dynamic segment present!");
		WriteLine("Warning: Assuming that the ELF-object contains no symbols!");
		*symbolCount = 0;
		*stringSize = 0;
	}

	struct DynamicEntry *dynEnts = (struct DynamicEntry *)
			((unsigned long) fileHdr + segmentEntry->fileOffset);

	*stringSize += FindEntryByTag(dynEnts, DT_STRSZ)->val;
	*symbolCount += ((unsigned long *)((unsigned long) fileHdr +
			FindEntryByTag(dynEnts, DT_HASH)->ptr))[1];
}

/**
 * Scans the segments of the given elf-object and calculates the highest
 * virtual address offset used in the program. This decides the amount of
 * space required by the program on being loaded.
 *
 * @param fileHdr - the elf-header of the object being parsed
 */
unsigned long ScanSegments(struct ElfHeader *fileHdr)
{
	struct ProgramHeader *segmentEntry = (struct ProgramHeader *)
			((unsigned long) fileHdr + fileHdr->programHeaderOffset);
	unsigned long segEntryCount = fileHdr->programHeaderEntryCount;
	unsigned long highestAddress = 0, segLimit;

	for(unsigned long index = 0;
			index < segEntryCount;
			index++, segmentEntry++) {
		if(segmentEntry->entryType != PT_LOAD)
			continue;

		segLimit = segmentEntry->virtualAddress + segmentEntry->memorySize;
		if(segLimit > highestAddress)
			highestAddress = segLimit;
	}

	return (PAGES_SIZE(highestAddress));
}
