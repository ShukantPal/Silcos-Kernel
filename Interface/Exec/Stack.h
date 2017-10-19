/**
 * Copyright (C) 2017 - Shukant Pal
 *
 * A stack is not just used by threads, but can also be used by extended
 * types (created ontop of KRUNNABLE). Thus, its functionalities must be
 * abstracted below the THREAD.
 *
 * Here, we are referring to processor stacks (not a data structure).
 */
#ifndef EXEC_STACK_H
#define EXEC_STACK_H

#include <TYPE.h>

#define PInitStack ((UINT32_T) 3 * 1024 * 1024 * 1024)

/**
 * KSTACKINFO - 
 *
 * Summary:
 * This type refers to the processor stack (esp) used by execution paths. It
 * stores the crucial information required to extend the stack.
 *
 * Fields:
 * Base - Base of the stack (from where it was allocated)
 * Pointer - Current (top) ptr to stack
 *
 * @Before STACK
 * @Version 1
 * @Since Circuit 2.01
 * @Author Shukant Pal
 */
typedef
struct _KSTACKINFO
{
	ULONG Base;
	ULONG Pointer;
} KSTACKINFO;

#endif /* Exec/Stack.h */
