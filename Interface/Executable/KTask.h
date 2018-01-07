/**
 * File: Exec.h
 *
 * Summary:
 * This file interfaces with the scheduler subsystem in the kernel. You can implement various types
 * of executable tasks using the KTASK type.
 *
 * Copyright (C) 2017 - Shukant Pal
 */
#ifndef EXEC_EXEC_H
#define EXEC_EXEC_H

#include <Executable/CPUStack.h>
#include <Memory/Pager.h>
#include <Types.h>
#include <Util/AVLTree.h>
#include <Util/LinkedList.h>

#include <Memory/Pager.h>

typedef unsigned long ID;
typedef unsigned long TYPE;
typedef unsigned short GID;
typedef unsigned short UID;
typedef U64 TIME;

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

enum {
	Runner_Executing = 0xFF1,
	Runner_Runnable = 0xFF2,
	Runner_Blocked = 0xFF3
};

#define TF_RAN 0
#define TF_KERNEL 1
#define TF_ROUTINE 2

typedef struct Processor Processor;

/**
 * Struct: KTask
 *
 * Summary:
 * Provides a homogenous-struct for maintaining records of executable entities
 * in the system. It is used by all schedulers to keep record of execution
 * status of any task (thread, deferred interrupt, kernel routine, group of
 * threads, user-mode scheduler activations, etc.)
 *
 * @Before EXEC
 * @Version 2.3
 * @Since Circuit 2.01
 * @Author Shukant Pal
 */
typedef
struct KTask
{
	union
	{
		AVLNODE Node;
		LinkedListNode ListLinker;
		struct
		{
			KTask *next;
			KTask *last;
			unsigned long NodeHeight;
			unsigned long Relativity;/* Task priority if required */
		};
	};
	void *eip;// instruction-pointer
	void (*run)(Processor *);// specialized run() method, null for default
	unsigned long taskFlags;// runtime execution flags
	CPUStack *userStack;// user-stack struct
	CPUStack *kernelStack;// kernel-stack struct (compulsory)
	LinkedList *ownerList;
	unsigned long RRM_ID;// resource-management id (deprecated)
	ID id;// unique id
	TYPE type;/* Legacy, type back-specifier */
	Processor *cpu;/* Processor holding the runqueue in which this task exists */
	CONTEXT *mmu;/* Address space which the task is using */
	TIME startTime;/* K-Time start */
	TIME timeStamp;/* K-Time last executed */
} EXEC; // 28/56 + 16/32 byte header for KRUNNABLE

typedef EXEC KRUNNABLE;
typedef EXEC KTASK;

static inline void SetupExec(KTask *e, ID ID, TYPE Type)
{
	e->id = ID;
	e->type = Type;
}

/**
 * KiSetupRunnable() - 
 *
 * Summary:
 * This function initializes the KRUNNABLE with the given data.
 *
 * Args:
 * kRunner - KRUNNABLE ptr
 * rID - Runner ID
 * rType - Runner type (object-type)
 *
 * @Version 1
 * @Since Circuit 2.03
 * @Author Shukant Pal
 */
static inline void KiSetupRunnable(KTask *kRunner, ID rID, TYPE rType){
	kRunner->id = rID;
	kRunner->type = rType;
	kRunner->eip = NULL;
	kRunner->run = NULL;
}

static inline KTask *SetupPtr(KTask *ptr, KTask *e)
{
	ptr->id = e->id;
	ptr->type = e->type;

	return (ptr);
}

#endif/* Exec/Exec.h */
