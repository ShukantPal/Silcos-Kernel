/**
 * Copyright (C) 2017 - Sukant Pal
 */

#ifndef TYPES_H
#define TYPES_H

#define import_asm extern // compile-time specifier
#define returnv_if(boolCondition) if(boolCondition) return;
#define return_if(boolCondition, retValue) if(boolCondition) return (retValue);

	#define VIRTUAL(ptr) ((unsigned long) ptr + 0xc0000000)
	#define PHYSICAL(ptr) ((unsigned long) ptr - 0xc0000000)

	#define _3_GB ((uint32_t) 3 * 1024*1024*1024)

	#define x86
	#define arch_ arch/x86
	#define IA32// Compulsory Add=on to x86

	#ifdef x64
		#define ARCH_64
		typedef unsigned char UBYTE;
		typedef unsigned char U8;
		typedef unsigned short U16;
		typedef unsigned int U32;
		typedef unsigned long U64;

		typedef signed char S8;
		typedef signed short S16;
		typedef signed int S32;
		typedef signed long S64;

		#define BITS 64
		#define BYTES_PER_LONG 8
		#define BITS_PER_LONG 64

		#define ULONG_OFFSET 3

		#define BLOCK_SIZE 4096
		#define BLOCK_DATA uint32_t
	#else
		#define ARCH_32
		typedef unsigned char UBYTE;
		typedef unsigned char U8;
		typedef unsigned short U16;
		typedef unsigned int U32;
		typedef unsigned long long U64;

		typedef signed char S8;
		typedef signed short S16;
		typedef signed int S32;
		typedef signed long S64;

		#define BITS 32

		#define BITS_PER_INT 32
		#define BYTES_PER_LONG 4
		#define BITS_PER_LONG 32

		#define ULONG_OFFSET 2

		#define BLOCK_SIZE 4096
		#define BLOCK_DATA uint32_t
	#endif

	#define size_t unsigned long
	#define SIZE_T unsigned long

	#define signed_size_t long
	#define SSIZE_T signed long

	#define VOID void

	#define NULL 0

	#define TRUE 1
	#define FALSE 0

	#define DEBUG

	#define sysc void 

typedef char CHAR;
typedef short SHORT;
typedef int INT;
typedef long LONG;
typedef unsigned char UCHAR;
typedef unsigned short USHORT;
typedef unsigned int UINT;
typedef unsigned long ULONG;
typedef unsigned long long ULONGLONG;
typedef unsigned long SIZE;
typedef UCHAR BOOL;

extern UINT KernelStart;
extern UINT KernelEnd;
extern ULONG StackAddress;
extern CHAR *HALData;

#endif /* Types.hpp */
