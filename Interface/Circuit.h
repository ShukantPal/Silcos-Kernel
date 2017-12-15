/**
 * Copyright (C) 2017 - Shukant Pal
 */

/**
 * Mainly, all definitions have been moved
 * to Util/.h files.
 */

#ifndef CIRCUIT_H
#define CIRCUIT_H

#include <Debugging.h>
#include <Types.h>

#define IsEven(n) !(n % 2)
#define IsOdd(n) (n % 2)

#define Max(A, B) ((A > B) ? A : B)

#define ScheduleFailedError 0xA1 /* init process lost */
#define OutOfMemoryError 0xA2 /* pmm alloc failed */
#define FragmenationError 0xA3 /* memory available (but cannot be allocated by pmm) */
#define HeapCorruptionError 0xA4 /* heap attribute corrupted */
#define UnknownInternalError 0xA5 /* unknown error (kernel) */
#define KernelPageFaultError 0xA6 /* page fault in kernel area */
#define CpuFeatureUnavailableError 0xA7 /* required feature unavailable (on old cpu) */

#define KernelBusError 0xB1 /* pci bus err during kernel init */
#define SystemServiceUnfoundError 0xB2 /* system service not found */


#define CastPtrOfStruct(type_name, var) (struct type_name*) var
#define Cast(Type, Variable) ((Type) Variable)

#define PtrAddress(P) (SIZE) P

void ASSERT(bool Condition, char *ErroMsg);

#endif /* Circuit.h */
