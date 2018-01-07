/**
 * Copyright (C) 2017 - Shukant Pal
 */

#ifndef ARCH_X86_PAGE_EXPLORER_H
#define ARCH_X86_PAGE_EXPLORER_H

#include <Memory/Address.h>
#include <Memory/Pager.h>

import_asm void SwitchPaging(unsigned int);
import_asm U64 PDPT[4];
import_asm U64 GlobalDirectory[512];
import_asm U64 IdentityDirectory[512];
import_asm U64 GlobalTable[512];

U64 *GetDirectory(unsigned long DirNo, unsigned long frFlags, CONTEXT *Context) kxhide;
U64 *GetPageTable(unsigned short DirNo, unsigned short TableNo,
			unsigned long frFlags, CONTEXT *Context) kxhide;

#endif /* Arch/x86/PageExplorer.h */
