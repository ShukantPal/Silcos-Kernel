/**
 * Copyright (C) 2017 - Shukant Pal
 */

#ifndef MEMORY_PAGE_TRANS_H
#define MEMORY_PAGE_TRANS_H

#include <Types.h>

#define PagePresent (1 << 0)
#define PageReadWrite (1 << 1)
#define PageUserland (1 << 2)
#define PageCacheDisable (1 << 3)

#ifdef NS_PMFLGS
	#define PRESENT		(1 << 0)
	#define READ_WRITE		(1 << 1)
	#define USER_MODE		(1 << 2)
	#define CACHE_DISABLE	(1 << 3)
#endif

#define PgPresentBTI 0
#define PgRWBTI 1
#define PgUserLdBTI 2
#define PgCheDisableBTI 3

typedef ULONG VIRTUAL_T;
typedef U64 PHYSICAL_T;
typedef U64 PADDRESS;
typedef U64 PAGE_ATTRIBUTES;

typedef ULONG ADDRESS;

#define PDPT(ptran) (ptran -> HardwarePage.PDPT)

typedef
struct PageTrans {
	UINT Padding[6]; /* Provide 32-byte padding for PDPT. */
	ULONGLONG PDPT[4];
} PAGE_TRANSALATOR;

#endif/* Memory/PageTrans.h */
