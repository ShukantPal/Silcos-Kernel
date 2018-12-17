/**
 * @file EarlyMultiboot.h
 *
 * Present few functions required to access information in the multiboot
 * information table.
 *
 * @version 1.0
 * @since Silcos 3.05
 * @author Shukant Pal
 */
#ifndef INITIALIZER_HEADERS_GENERIC_EARLYMULTIBOOT_H_
#define INITIALIZER_HEADERS_GENERIC_EARLYMULTIBOOT_H_

/* Assumes that _CBUILD has already been declared. */
#include <Multiboot2.h>

static inline void FetchAdjacentTag(union MultibootSearch *genPtr)
{
	genPtr->loc += (genPtr->tag->size % 8) ?
					genPtr->tag->size + 8 - genPtr->tag->size % 8 :
					genPtr->tag->size;
}

void InitMultiboot2Access(unsigned long accessPointer);
union MultibootSearch FindTagByType(unsigned long tt);

void ScanAllModules(unsigned long *moduleCount, unsigned long *segmentSpace,
		unsigned long *symbols, unsigned long *strings);
void ScanAllModules2();
unsigned long FindEnvSpace();
unsigned FindHostIndex();

#endif/* Generic/EarlyMultiboot.h */
