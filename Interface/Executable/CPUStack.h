#ifndef EXEC_STACK_H
#define EXEC_STACK_H

#include <TYPE.h>

#define PInitStack ((UINT32_T) 3 * 1024 * 1024 * 1024)

struct CPUStack
{
	unsigned long base;
	unsigned long pointer;
};

#endif /* Exec/Stack.h */
