/*=++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * File: MemoryTransfer.h
 *
 * Summary: This file contains the interface to map physical pageframes from one virtual address
 * to another. This is useful because copying is not needed, and thus data structures can easily be
 * expanded.
 *
 * Functions:
 *
 * GetFrames() - This function is used for retrieving physical pageframes from a virtual address.
 *
 * SetFrames() - This function is used for mapping the pageframes, given from GetFrames() to the
 * destination address.
 *
 * Copyright (C) 2017 - Shukant Pal
 *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=*/
#ifndef MEMORY_TRANSFER_H
#define MEMORY_TRANSFER_H

#define NS_KFRAMEMANAGER/* MM_FRAME is internal to the KFrameManager */

#include "Pager.h"
#include "KFrameManager.h"

MMFRAME *GetFrames(ADDRESS address, ADDRESS pages, CONTEXT *pageContext);
void SetFrames(ADDRESS address, MMFRAME *frameHead, CONTEXT *pageContext);

#endif
