/**
 * Copyright (C) - Shukant Pal
 */

#ifndef MESSAGE_PASSING_H
#define MESSAGE_PASSING_H

#include <Executable/Executable::KTask.h>

#include "../Utils/AVLTree.hpp"
#include "../Utils/LinkedList.h"
#include "IPC.h"


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
	void *Buffer;
} MESSAGE;

typedef
struct _MESSAGE_QUEUE
{
	IPC_HEADER Control;
	AVL_TREE PriorityMsgTree;
	unsigned long UsageLimit;
	unsigned long SizeLimit;
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

long ReadMsg(
	ID queueID,
	void *bufferPointer,
	unsigned short bufferSize,
	unsigned short msgTag,	
	unsigned short minPriority,
	unsigned long msgFlags,
	unsigned short timeOut
);

long SendMsg(
	ID queueID,
	void *bufferPointer,
	unsigned short bufferSize,
	unsigned short msgTag,
	unsigned short msgPriority,
	unsigned long msgFlags,
	unsigned short timeOut
);

#endif
