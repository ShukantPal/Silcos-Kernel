/**
 * Copyright (C) 2017 - Shukant Pal
 * 
 * All programs require resources (given by the system), and to manage these
 * resources, the system abstracts them into processes. Here, a process is just
 * a resource-holder (along with some more things).
 *
 * It can be scheduled by using a process-scheduler, but that is a rarely used
 * feature.
 *
 * A process is also a holder of the 'root' context, or the context containing
 * the program core (elf sections), but threads can switch address spaces by
 * controlled means like 'LPC', where the stacks are same, but program core is
 * different.
 */
#ifndef EXEC_PROCESS_H
#define EXEC_PROCESS_H

#include <Exec/KTask.h>
#include <HAL/Processor.h>
#include <Memory/Pager.h>
#include <Memory/PMemoryManager.h>
#include <Memory/KMemorySpace.h>
#include <Synch/Spinlock.h>
#include <Types.h>

enum
{
	PRunning = 0x1,
	PRunnable = 0x2,
	PWaiting = 0x3,
	PSleeping = 0x4,
	PTerminated = 0x5
};

/* New state macros */
enum
{
	Process_Running = 11,  // The running & runnable states are unique
	Process_Runnable = 13, // in having the first bit set.
	Process_Waiting = 0x2, 
	Process_Sleeping = 0x4,
	Process_Blocked = 0x6,
	Process_Terminated = 0x8
};

typedef
struct EProcessInfo
{
	TIME Sleeptime; /* Time till waking up. */
	TIME Waittime; /* Maximum time for process to wait for. */
//	PList Waiting; /* Processes waiting on this one */
} EPI;

typedef struct EProcessInfo PROCESS_RELATIONS;

#define PID(P) (P -> Gate.ID)
#define Prio(P) (P -> Priority)
#define Priv(P) (P -> Privelege)
#define GID(P) (P -> GroupId)
#define UID(P) (P -> UserId)
#define State(P) (P -> State)
#define EPI(P) (P -> ExPI)
#define Context(P) (P -> AddressSpace)
#define Anatomics(P) (P -> Anatomics)

#define PFlags(P) (P -> Flags)
#define IsUserMode(P) (P -> Flags & 1)

/**
 * KPROCESS - 
 *
 * Summary:
 * This type represents a resource-holder, namely - the PROCESS. It is a parent
 * for its scheduling units and owns a address space, priority, and IPC context.
 *
 * A process is schedulable (by means of process-scheduler), for usage of sub-
 * scheduling techniques.
 *
 * A process is identified mainly by its memory manager (PMM_MANAGER), which
 * should be appreiciated for its generality in referring to processes. But, (some)
 * special programs require special memory managers, and need a different
 * resource-holder type.
 *
 * Fields:
 * Gate - KRUNNABLE interface
 * Priority - Resource-priority
 * Privelege - System-based priveleges and type-identifiers
 * GroupId - Humanware identifier
 * UserId - Program-client identifier
 * Flags - Resource-flags
 * MemoryManager - PSMM-compatible memory manager (for PAS-recruitment)
 * AddressSpace
 * ExPI - External affairs
 *
 * @Before PROCESS
 * @Version 2
 * @Since Circuit 2.01
 * @Author Shukant Pal
 */
typedef
struct _KPROCESS {
	union {
		KRUNNABLE Gate;
		struct {
			LIST_ELEMENT LiLinker;
			ID PID;
		};
	};
	UBYTE Priority;
	UBYTE Privelege;
	GID GroupId;
	UID UserId;
	unsigned short State;
	unsigned long Flags;
	PSMM_MANAGER *MemoryManager;
	CONTEXT *AddressSpace;
	EPI ExPI;
} KPROCESS;

typedef unsigned long KPROCESS_HANDLE;

extern KPROCESS *psKernel;/* psKernel process for containing non-core kernel processes */

/**
 * KeGetProcess() - 
 *
 * Summary:
 * This function validates the PID and then returns the process being referred
 * to by this PID.
 *
 * Args: 
 * processID - PID of the referred process
 *
 * Returns: KPROCESS structure (pointer)
 *
 * @Version 1
 * @Since Circuit 2.03
 * @Author Shukant Pal
 */
KPROCESS *KeGetProcess(
	ID processID
);

void InitPTable(void);

/**
 * KeNewProcess() -
 *
 * Summary:
 * This function is used for creating a new process, with the required fields. A process
 * can be created on behalf of a process only (not a thread).
 *
 * 
 */
void XNewProcess();
void XCopyProcess(void);
void XExecProcess(void);
void XExitProcess(void);
void XDeleteProcess(void);
void XWaitForProcess(void);
void XSleepProcess(void);
void XSuspendProcess(void);
void XWakeupProcess(void);

#endif /* Exec/Process.h */
