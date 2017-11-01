/* Copyright (C) 2017 - Shukant Pal */

#include <Memory/KMemorySpace.h>
#include <Memory/Pager.h>
#include <Multiboot.h>
#include <Multiboot2.h>
#include <Debugging.h>

U32 tagTableSize;
MULTIBOOT_TAG *tagTable;
U32 t = 32;

export_asm Void *SearchMultibootTag(U32 tagType){
	MULTIBOOT_TAG *curTag = tagTable + 1;
	U32 tagOffset;
	U32 tagLimit = (U32) tagTable + tagTableSize;

	U32 counter = 0;
	while((U32) curTag < tagLimit){
		if(curTag->Type == tagType) {
			return (VOID*) (curTag);
		}

		tagOffset = curTag->Size;
		if(tagOffset % 8)
			tagOffset += 8 - (tagOffset % 8);

		curTag = (MULTIBOOT_TAG*) ((U32) curTag + tagOffset);
	}

	return (NULL);
}

export_asm Void *SearchMultibootTagFrom(Void *lastTag, U32 tagType){
	MULTIBOOT_TAG *curTag;
	U32 tagOffset;

	if(lastTag == NULL)
		curTag = tagTable + 1;
	else {
		curTag = lastTag;
		tagOffset = curTag->Size;
		if(tagOffset % 8)
			tagOffset += 8 - (tagOffset % 8);
		curTag = (MULTIBOOT_TAG*) ((U32) curTag + tagOffset);
	}

	U32 tagLimit = (U32) tagTable + tagTableSize;	

	while((U32) curTag < tagLimit){
		if(curTag->Type == tagType){
			return (VOID*) (curTag);
		}

		tagOffset = curTag->Size;
		if(tagOffset % 8)
			tagOffset += 8 - (tagOffset % 8);

		curTag = (MULTIBOOT_TAG*) ((U32) curTag + tagOffset);
	}

	return (NULL);
}

/**
 * LoadMultibootTags() -
 *
 * Summary:
 * This function maps the multiboot tags at the specific address
 * of MULTIBOOT_INTERFACE and ensures their validity.
 *
 * Args:
 * tagTable - Physical address of the tag-table
 *
 * @Version 1
 * @Since Circuit 2.03
 */
export_asm void LoadMultibootTags(U32 pTagAddress){
	InitConsole((UCHAR *) 0xc00b8000);

	SwitchContext(&SystemCxt);
	EnsureMapping(MULTIBOOT_INTERFACE, (PADDRESS) pTagAddress, NULL, 0, KernelData);

	tagTable = (MULTIBOOT_TAG*) (MULTIBOOT_INTERFACE + (pTagAddress % KB(4)));
	tagTableSize = *((U32*) tagTable);
}
