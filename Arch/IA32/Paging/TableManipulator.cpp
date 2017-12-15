/**
 * Copyright (C) 2017 - Shukant Pal
 */

#include <IA32/PageExplorer.h>
#include <Memory/KFrameManager.h>

U64 *GetDirectory(unsigned long dirOffset, unsigned long frFlags, CONTEXT *Context){
	if(dirOffset == 3)
		return (GlobalDirectory);

	if(!(PDPT(Context)[dirOffset] & 1)) {
		PDPT(Context)[dirOffset] = (U64) KiFrameEntrap(frFlags) | 1;
		GlobalTable[508 + dirOffset] = PDPT(Context)[dirOffset] | 2;
		GlobalDirectory[508 + dirOffset] = PDPT(Context)[dirOffset] | 2;
	}

	return (U64*) (GB(3) + MB(2 * 511) + KB(4 * (508 + dirOffset)));
}

U64 *GetPageTable(unsigned short dirOffset, unsigned short tableOffset, unsigned long frFlags, CONTEXT *Context){
	U64 *pgDirectory = GetDirectory(dirOffset, frFlags, Context);
	if(dirOffset == 3){
		if(!(pgDirectory[tableOffset] & 1)){
			pgDirectory[tableOffset] = (U64) KiFrameEntrap(frFlags) | 3;
		}

		return (U64*) (GB(3) + MB(507 * 2) + KB(tableOffset * 4));
	} else {
		if(!pgDirectory[tableOffset] & 1){
			if(!(pgDirectory[tableOffset] >> 7 & 1))
				pgDirectory[tableOffset] = (U64) KiFrameEntrap(frFlags) | 1;
			else
				return (NULL);
		}

		return (U64*) (GB(3) + MB((508 + dirOffset) * 2) + KB(tableOffset * 4));
	}
}
