/**
 * @file BinaryAnalyzer.h
 *
 * Provides various functions to pre-fetch size requirements for loading
 * various files with different binary formats.
 *
 *  Created on: 13-Jun-2018
 *      Author: sukantdev
 */
#ifndef INITIALIZER_HEADERS_GENERIC_BINARYANALYZER_H_
#define INITIALIZER_HEADERS_GENERIC_BINARYANALYZER_H_

#include <Module/Elf/ELF.h>

bool IsElfFile(void *file);
void ScanSymbols(struct ElfHeader *fileHdr, unsigned long *symbolCount,
		unsigned long *stringSize);
unsigned long ScanSegments(struct ElfHeader *fileHdr);
unsigned long ScanPoolSize(struct ElfHeader *fileHdr);

#endif/* Generic/BinaryAnalyzer.h */
