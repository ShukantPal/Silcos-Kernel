/**
 * Copyright (C) 2017 - Shukant Pal
 */

#ifndef ARCH_X86_PAGE_EXPLORER_H
#define ARCH_X86_PAGE_EXPLORER_H

#include <Circuit.h>
#include <Memory/Address.h>
#include <Memory/Pager.h>

import_asm VOID SwitchPaging(UINT);
import_asm U64 PDPT[4];
import_asm U64 GlobalDirectory[512];
import_asm U64 IdentityDirectory[512];
import_asm U64 GlobalTable[512];

U64 *GetDirectory(
	ULONG DirNo,
	ULONG frFlags,
	CONTEXT *Context
);

U64 *GetPageTable(
	USHORT DirNo,
	USHORT TableNo,
	ULONG frFlags,
	CONTEXT *Context
);

#endif /* Arch/x86/PageExplorer.h */
