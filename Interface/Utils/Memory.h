/**
 * Copyright (C) 2017 - Shukant Pal
 */
#ifndef UTIL_MEMORY_H
#define UTIL_MEMORY_H

#include "../Utils/CtPrim.h"

template<class SwapClient>
void swapObjects(SwapClient *swc0, SwapClient *swc1)
{
	union
	{
		unsigned char dumbByteBuffer[sizeof(SwapClient)];
		SwapClient localCopy;
	};

	memcpyf(reinterpret_cast<void*>(swc0), (void*) &dumbByteBuffer,
			sizeof(SwapClient));

	memcpyf(reinterpret_cast<void*>(swc1), reinterpret_cast<void*>(swc0),
			sizeof(SwapClient));

	memcpyf(reinterpret_cast<void*>(&localCopy),
			reinterpret_cast<void*>(swc1), sizeof(SwapClient));
}

#endif
