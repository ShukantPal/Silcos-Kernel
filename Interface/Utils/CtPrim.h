/**
 * Copyright (C) - Shukant Pal
 */

#ifndef UTIL_CT_PRIM_H
#define UTIL_CT_PRIM_H

#include <Types.h>

/* mem-management functions (primitive & independent) */
void memset(void *Pointer, unsigned char Value, SIZE Size);
void memsetf(void *Pointer, SIZE Value, SIZE ByteSize);
void memcpy(const void *Pointer, void *Buffer, SIZE PointerAndBufferSize);
void memcpyf(const void *Pointer, void *Buffer, SIZE ByteSize);

#define MEMORY_SET memset
#define MEMORY_SET_F memsetf

bool memcmp(const void *from, const void *to, SIZE cpSize);
bool strcmp(const char *str0, const char *str1);
bool strcmpn(const char *str0, const char *str1, const long n);

#endif/* Util/CtPrim.h */