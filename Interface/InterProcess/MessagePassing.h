/**
 * Copyright (C) - Shukant Pal
 */

#ifndef MESSAGE_PASSING_H
#define MESSAGE_PASSING_H

#include <Exec/Exec.h>
#include "IPC.h"

#include <Util/AVLTree.h>
#include <Util/LinkedList.h>

#define UPPER_LIMIT_MSG_BUFFER_SIZE 4096
#define MSG_TAG_ANY 0
#define MSG_NO_PRIORITY 0

#ifdef NAMESPACE_MESSAGE_PASSING_INTERNAL

typedef
struct _MESSAGE
{
	LIST_ELEMENT MsgLinker;
	ID Sender;
	SIZE Size;
	VOID *Buffer;
} MESSAGE;

typedef
struct _MESSAGE_QUEUE
{
	IPC_HEADER Control;
	AVL_TREE PriorityMsgTree;
	ULONG UsageLimit;
	ULONG SizeLimit;
	ID LastSender;
	ID LastReader;
	TIME LastSend;
	TIME LastRead;
	TIME LastChange;
} MESSAGE_QUEUE;

#else

struct _MESSAGE;
struct _MESSAGE_QUEUE;
typedef struct _MESSAGE MESSAGE;
typedef struct _MESSAGE_QUEUE MESSAGE_QUEUE;

#endif

ID CreateMsgQueue(
	SIZE queueMaxBytes,
	SIZE queueMaxMessages
);

LONG ReadMsg(
	ID queueID,
	VOID *bufferPointer,
	USHORT bufferSize,
	USHORT msgTag,	
	USHORT minPriority,
	ULONG msgFlags,
	USHORT timeOut
);

LONG SendMsg(
	ID queueID,
	VOID *bufferPointer,
	USHORT bufferSize,
	USHORT msgTag,
	USHORT msgPriority,
	ULONG msgFlags,
	USHORT timeOut
);

#endif
