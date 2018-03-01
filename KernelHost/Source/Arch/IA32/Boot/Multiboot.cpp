/* Copyright (C) 2017 - Shukant Pal */

#include <Memory/KMemorySpace.h>
#include <Memory/Pager.h>
#include <Multiboot2.h>
#include <Debugging.h>

U32 tagTableSize;
MULTIBOOT_TAG *tagTable;

export_asm Void *SearchMultibootTagFrom(Void *lastTag, U32 tagType)
{
	MULTIBOOT_TAG *curTag;
	U32 tagOffset;

	if(lastTag == NULL)
		curTag = tagTable + 1;
	else {
		curTag = (MULTIBOOT_TAG*) lastTag;
		tagOffset = curTag->Size;
		if(tagOffset % 8)
			tagOffset += 8 - (tagOffset % 8);
		curTag = (MULTIBOOT_TAG*) ((U32) curTag + tagOffset);
	}

	U32 tagLimit = (U32) tagTable + tagTableSize;	

	while((U32) curTag < tagLimit){
		if(curTag->Type == tagType){
			return (void*) (curTag);
		}

		tagOffset = curTag->Size;
		if(tagOffset % 8)
			tagOffset += 8 - (tagOffset % 8);

		curTag = (MULTIBOOT_TAG*) ((U32) curTag + tagOffset);
	}

	return (NULL);
}

/*
 * Initializes the multiboot-parser. It should be called before main because
 * multiboot information passed in the CPU registers will be lost.
 *
 * @param tagTable - physical address of multiboot-information table
 * @version 1
 * @since Circuit 2.03
 * @author Shukant Pal
 */
export_asm void LoadMultibootTags(U32 pTagAddress)
{
	InitConsole((unsigned char *) 0xc00b8000);
	Pager::switchSpace(KERNEL_CONTEXT);

	Pager::map(MULTIBOOT_INTERFACE, (PhysAddr) pTagAddress, 0, KernelData);

	tagTable = (MULTIBOOT_TAG*)(MULTIBOOT_INTERFACE +
					(pTagAddress % KB(4)));
	tagTableSize = *((U32*) tagTable);
}
