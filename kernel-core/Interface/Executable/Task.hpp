/**
 * File: KTask.h
 *
 * Summary:
 * This file interfaces with the scheduler subsystem in the kernel. You can implement various types
 * of executable tasks using the Executable::KTask type.
 *
 * Copyright (C) 2017 - Shukant Pal
 */
#ifndef EXEC_EXEC_H
#define EXEC_EXEC_H

#include <Executable/CPUStack.h>
#include <Memory/Pager.h>
#include <Types.h>
#include <Memory/Pager.h>
#include "../Utils/AVLTree.hpp"
#include "../Utils/LinkedList.h"

typedef unsigned long ID;
typedef unsigned long TYPE;
typedef unsigned short GID;
typedef unsigned short UID;

/* Types for the Exec resources. */
enum
{
	ProcessTp = 0xA,
	ThreadTp = 0xB,
	ThreadGroupTp = 0xC,
	KernelRoutineTp = 0xD,
	ProcessGroupTp = 0xE,
	ExecPtrTp = 0xF
};

namespace HAL { struct Processor; }

namespace Executable
{
enum TaskState
{
	Start,
	Execute,
	Runnable,
	SleepInterruptible,
	SleepUninterruptible,
	Destroy
};

/**
 * Struct: KTask
 *
 * Summary:
 * Used for storing execution state of tasks pending in the system. It should
 * be embedded in other structs (e.g. Thread) which will provide more features
 * like stacks and dispatchers.
 *
 * Author: Shukant Pal
 */
struct Task
{
	union
	{
		AVLNode Node;
		LinkedListNode ListLinker;
		struct
		{
			Executable::Task *next;
			Executable::Task *last;
			unsigned long nodeHeight;
			unsigned long Relativity;/* Task priority if required */
		};
	};
	void *eip;// instruction-pointer
	void (*run)(HAL::Processor *);// specialized run() method, null for default
	unsigned long taskFlags;// runtime execution flags
	CPUStack *userStack;// user-stack struct
	CPUStack *kernelStack;// kernel-stack struct (compulsory, unless dispatcher is used)
	unsigned long schedClass;
	TaskState state;// resource-management id (deprecated)
	ID id;// unique id
	TYPE type;/* Legacy, type back-specifier */
	HAL::Processor *cpu;/* Processor holding the runqueue in which this task exists */
	CONTEXT *mmu;/* Address space which the task is using */
	Time startTime;/* K-Time start */
	Time timeStamp;/* K-Time last executed */

	union
	{
		AVLNode timeoutNode;
		struct
		{
			Task *left;
			Task *right;
			unsigned long timeoutHeight;
			Time wakeupTime;
		};
	};

	void kill();
	void sleep(Time waitPeriod);
	void wakeup();
}; // 28/56 + 16/32 byte header for Executable::KTask

}

decl_c void AddTaskToTimeout();// called ONLY ON boot-strap processor
decl_c void RemoveTaskFromTimeout();// called ONLY ON boot-strap processor

static inline void KiSetupRunnable(Executable::Task *kRunner, ID rID, TYPE rType)
{
	kRunner->id = rID;
	kRunner->type = rType;
	kRunner->eip = NULL;
	kRunner->run = NULL;
}

#endif/* Executable/Executable::KTask.h */
