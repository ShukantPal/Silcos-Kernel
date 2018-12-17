/**
 * Copyright (C) 2017 - Sukant Pal
 */

#ifndef __TYPES_H__
#define __TYPES_H__

#define x86
#define arch_ arch/x86
#define IA32// Compulsory Add=on to x86

#ifdef IA32
	typedef unsigned long size_t;
#elif IA64
	typedef unsigned int size_t;
#endif

#ifndef _CBUILD
	#define import_asm extern "C" // compile-time specifier
	#define export_asm extern "C"
	#define decl_c extern "C"

	extern struct ObjectInfo *tLinkedList;
	inline void *operator new(size_t mem_size, void *objectMemory)
	{
		return (objectMemory);
	}

	extern "C" void *KNew(struct ObjectInfo *, unsigned long km_sleep);
	inline void *operator new(size_t obj_size, struct ObjectInfo *objectType)
	{
		return (KNew(objectType, 0));
	}

	typedef void kobj;
	extern "C" void KDelete(void *del_ptr, struct ObjectInfo *);
	static inline void kobj_free(void *del_ptr, struct ObjectInfo *useless_type)
	{
		KDelete(del_ptr, useless_type);
	}

#else
	#define import_asm extern
	#define export_asm extern
	#define decl_c extern

	#define bool int
	#define true 1
	#define false 0
#endif

#define returnv_if(boolCondition) if(boolCondition) return;
#define return_if(boolCondition, retValue) if(boolCondition) return (retValue);

	#define VIRTUAL(ptr) ((unsigned long) ptr + 0xc0000000)
	#define PHYSICAL(ptr) ((unsigned long) ptr - 0xc0000000)

	#define _3_GB ((uint32_t) 3 * 1024*1024*1024)

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

	#define signed_size_t long
	#define SSIZE_T signed long

	#define NULL 0

	#define TRUE 1
	#define FALSE 0

	#define DEBUG

	#define sysc void 

#define getAddress(localReference)((unsigned long)(&localReference))

typedef unsigned long SIZE;
typedef void Void;
typedef SIZE Size;

typedef char byte;

#define null 0

//!
//! @code strong_null macro is used at places where null can be confused with
//! another object that behave like null, with explicit properties (like the
//! rb-tree's nil node). This tells the reader that the programmer intended to
//! pass null and not the placebo null-object.
//!
#define strong_null 0

//!
//! @code weak_null means that the object at address 0 is being passed. It
//! doesn't mean no object exists. Rarely used, like in shared-memory
//! addressing where relative offset addresses are used.
//!
#define weak_null 0

typedef unsigned char uchar;
typedef unsigned char ubyte;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;

extern unsigned int KernelStart;
extern unsigned int KernelEnd;
extern unsigned int KernelCodeStart;
extern unsigned int KernelCodeEnd;
extern unsigned int KernelDataStart;
extern unsigned int KernelDataEnd;
extern unsigned int KernelBSSStart;
extern unsigned int KernelBSSEnd;
extern unsigned int KernelPDatStart;
extern unsigned int KernelPDatEnd;
extern unsigned long StackAddress;
extern char *HALData;

#ifndef _CBUILD
extern void ImmatureHang(const char*);
#endif/* _CBUILD */

#endif /* Types.hpp */
