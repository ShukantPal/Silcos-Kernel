/**
 * Copyright (C) - Shukant Pal
 */

#ifndef UTIL_CT_PRIM_H
#define UTIL_CT_PRIM_H

#include <Types.h>

/* mem-management functions (primitive & independent) */
VOID memset(
	VOID *Pointer,
	UCHAR Value,
	SIZE_T Size
);

VOID memsetf(
	VOID *Pointer,
	SIZE_T Value,
	SIZE_T ByteSize
);

VOID memcpy(
	const VOID *Pointer,
	VOID *Buffer,
	SIZE_T PointerAndBufferSize
);

VOID memcpyf(
	const VOID *Pointer,
	VOID *Buffer,
	SIZE_T ByteSize
);

#define MEMORY_SET memset
#define MEMORY_SET_F memsetf

BOOL memcmp(
	const VOID *Pointer0,
	const VOID *Pointer1,
	SIZE_T CmpByteSize
);

BOOL strcmp(
	const CHAR *String0,
	const CHAR *String1
);

#endif /* Util/CtPrim.h */
