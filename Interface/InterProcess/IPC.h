/**
 * Copyright (C) - Shukant Pal
 */

#ifndef IPC_H
#define IPC_H

#include <Types.h>

enum 
{
	IPC_CREAT = 0,
	IPC_EXCL = 1,
	IPC_NOWAIT = 2
};

enum
{
	IPC_Futex = 0,
	IPC_MessageQueue = 1,
	IPC_Mutex = 2,
	IPC_Semaphore = 3,
	IPC_SharedMemory = 4
};

typedef
struct _IPC_HEADER
{
	U64 Signature;
	ULONG OwnerUID;
	ULONG OwnerGID;
	ULONG CreatorUID;
	ULONG CreatorGID;
} IPC_HEADER;

#ifdef NAMESPACE_IPC_MANAGEMENT
	#define DefaultSignature 0xff87e239ab30845c
#endif /* NAMESPACE_IPC_MANAGEMENT */

#endif /* InterProcess/IPC.h */
