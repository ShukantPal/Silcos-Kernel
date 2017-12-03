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

#include "Stack.h"
#include <Memory/Pager.h>
#include <Types.h>
#include <Util/AVLTree.h>
#include <Util/LinkedList.h>

#include <Memory/Pager.h>

typedef ULONG ID;
typedef ULONG TYPE;
typedef USHORT GID;
typedef USHORT UID;
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

/* Helper macros */
#define EID(Ex) (Ex -> ID)
#define ETypeInfo(Ex) (Ex -> Type)
#define NextSched(Ex) (Ex -> ListLinker.Next)
#define PrevSched(Ex) (Ex -> ListLinker.Previous)

#define TF_RAN 0
#define TF_KERNEL 1
#define TF_ROUTINE 2

typedef
struct _PROCESSOR
PROCESSOR;

/**
 * KRUNNABLE -
 *
 * Summary:
 * A KTASK or kernel-task is a object that represents a task in the system that is undergoing scheduling. It replaces
 * the thread as the basic scheduling unit so that other tasks like DPCs, LPCs, KRUNs, and KTHREADs can be scheduled
 * homogenously.
 *
 * @Before EXEC
 * @Version 2.3
 * @Since Circuit 2.01
 * @Author Shukant Pal
 */
typedef
struct Exec {
	union {
		AVLNODE Node;
		LIST_ELEMENT ListLinker;
		struct {
			struct Exec *RightLinker;
			struct Exec *LeftLinker;
			ULONG NodeHeight;
			ULONG Relativity;/* Task priority if required */
		};
	};
	VOID *EIP;/* IP for this task */
	VOID (*Run)(PROCESSOR *);/* Task loader (sub-scheduler or NULL) */
	ULONG TaskFlags;/* Runtime flags */
	KSTACKINFO *UserStack;/* User-mode stack, if any */
	KSTACKINFO *KernelStack;/* Kernel-mode stack, required */
	LINKED_LIST *OwnerList;/* Legacy, list back-pointer */
	ULONG RRM_ID;/* Resource-management id */
	ID ID;/* TASK Id*/
	TYPE Type;/* Legacy, type back-specifier */
	PROCESSOR *Processor;/* Processor holding the runqueue in which this task exists */
	CONTEXT *Context;/* Address space which the task is using */
	TIME StartTime;/* K-Time start */
	TIME ExecutionTime;/* K-Time last executed */
} EXEC; // 28/56 + 16/32 byte header for KRUNNABLE

typedef EXEC KRUNNABLE;
typedef EXEC KTASK;

static inline
VOID SetupExec(EXEC *E, ID ID, TYPE Type)
{
	EID(E) = ID;
	ETypeInfo(E) = Type;
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
static inline
VOID KiSetupRunnable(KRUNNABLE *kRunner, ID rID, TYPE rType){
	kRunner->ID = rID;
	kRunner->Type = rType;
	kRunner->EIP = NULL;
	kRunner->Run = NULL;
}

static inline
EXEC *SetupPtr(EXEC *Ptr, EXEC *E)
{
	EID(Ptr) = EID(E);
	ETypeInfo(Ptr) = ETypeInfo(E);

	return (Ptr);
}

#endif /* Exec/Exec.h */
