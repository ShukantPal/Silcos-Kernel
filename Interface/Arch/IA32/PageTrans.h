/**
 * Copyright (C) 2017 - Shukant Pal
 */

#ifndef MEMORY_PAGE_TRANS_H
#define MEMORY_PAGE_TRANS_H

#include <TYPE.h>

#define PagePresent 		(1 << 0)
#define PageReadWrite 		(1 << 1)
#define PageUserland 		(1 << 2)
#define PageCacheDisable	(1 << 3)

#ifdef NS_PMFLGS
	#define PRESENT			(1 << 0)
	#define READ_WRITE		(1 << 1)
	#define USER_MODE		(1 << 2)
	#define CACHE_DISABLE	(1 << 3)
#endif

#define PgPresentBTI 0
#define PgRWBTI 1
#define PgUserLdBTI 2
#define PgCheDisableBTI 3

/*
 * Aligns the given memory address/size by trimming it page-aligned.
 */
static inline unsigned long getPageAligned(unsigned long bytes) {
	return (bytes & ~0xFFF);
}

/*
 * Aligns the given memory address/size by trimming it huge-page aligned.
 */
static inline unsigned long getHugePageAligned(unsigned long bytes) {
	return (bytes & ~0xFFFFF);
}

/*
 * Returns the size, in bytes, to the total number of pages
 * covered with the given address/amount of memory, assuming that it
 * starts on a page-aligned address.
 */
static inline unsigned long getPagesSize(unsigned long bytes) {
	return ((bytes % 0x1000) ?
			((bytes & ~0xFFF) + 0x1000) : bytes);
}

/*
 * Returns the size, in bytes, of the total number of huge pages
 * covered with the given address/amount of memory, assuming that it
 * starts on a huge-page aligned address.
 */
static inline unsigned long getHugePagesSize(unsigned long bytes) {
	return ((bytes % 0x1000000) ?
			((bytes & ~0xFFFFFF) + 0x1000000) : bytes);
}

typedef unsigned long ADDRESS;
typedef U64 PHYSICAL_T;
typedef U64 PhysAddr;
typedef U32 VirtAddr;
typedef U64 PAGE_ATTRIBUTES;
typedef U64 PageAttributes;

typedef unsigned long ADDRESS;

#define PDPT(ptran)(ptran->HardwarePage.PDPT)

typedef
struct PageTrans
{
	unsigned int Padding[5]; /* Provide 32-byte padding for PDPT. */
	unsigned long physPDPTAddr;
	unsigned long long PDPT[4];

	U64 *getPDPT() {
		return (PDPT);
	}

} PAGE_TRANSALATOR;

static inline unsigned long getAddressFor(unsigned long pdptIndex,
		unsigned long dirIndex, unsigned long tableIndex) {
	return ((pdptIndex << 30) + (dirIndex << 21) + (tableIndex << 12));
}

extern "C" void EraseIdentityPage(Void);

#endif/* Memory/PageTrans.h */
