/**
 * Copyright (C) 2017 - Shukant Pal
 *
 * A thread is a basic scheduling unit in modern operating systems. Its importance is
 * lowered by using the KRUNNABLE interface to the scheduler, but it is must be given
 * to run user-application. Here, a thread is just a execution path inside a process that
 * can be run concurrently along with other threads (on different CPUs). But, threads
 * undergo IPC (not processes), and thus can switch address spaces (if allowed).
 *
 * A kThread belongs to the (Kernel) process. This process has no address space
 * and thus, kernel threads cannot rely on the data below KERNEL_OFFSET.
 */
#ifndef EXEC_THREAD_H
#define EXEC_THREAD_H

#include "Exec.h"
#include "SchedList.h"
#include "Stack.h"
#include <Memory/Pager.h>
#include <Synch/Spinlock.h>
#include <Types.h>

/* Gate macro */
#define TGate(T) (T -> Gate)
#define TdGate(T) (T -> Gate)

/* TdStatus macros */
enum
{
	Thread_Running = 0x1,
	Thread_Runnable = 0x2,
	Thread_Sleeping = 0x3,
	Thread_Waiting = 0x4,
	Thread_Blocked = 0x5,
	Thread_Terminated = 0x6,
	Thread_Waiting_Till = 0x7,
	Thread_Stopped = 0x8
};

/* TdFlags macros */
enum ThreadState
{
	Thread_Ran = (1 << 0),
	Thread_Global = (1 << 1),
	Thread_System = (1 << 2),
	Thread_Kernel = (1 << 3),
	Thread_CustomStack = (1 << 4)
};

struct EThreadInfo
{
	TIME SleepTime;
	EXEC *PtrConjoint;
	ExecList ExecWaiters;
};

typedef struct EThreadInfo ETI;

/* Helper macros */
#define LocalPrio(T) (T -> Priority)
#define LocalPriv(T) (T -> Privelege)

#define TdPrio(T) (T -> Priority)
#define TdPrivelege(T) (T -> Privelege)

#define TFlags(T) (T -> Flags)
#define TdFlags(T) (T -> Flags)
#define TdStatus(T) (T -> Status)

#define Status(T) (T -> Status)
#define ProgramCounter(T) (T -> ProgramCounter)
#define StackBase(T) (T -> UserStack.Base)
#define StackPtr(T) (T -> UserStack.Pointer)
#define ParentID(T) (T -> ParentID)

/* TFlags macros */
#define ISRealtime(T) (TFlags(T) & (1 << 0))
#define ISUserMode(T) (TFlags(T) & (1 << 1))

/* User-mode & kernel-mode threads */

/* User-mode & kernel-mode values*/
#define UserThread 1
#define KernelThread 0

/* Thread in user-mode or in kernel-mode. */
#define ThreadInUser(T) ((TFlags(T) >> 1) & 1)
#define ThreadInKernel(T) (!ThreadInUser(T))

#define CustomStack(T) (TdFlags(T) >> 4 & 1)

/* Also #include <Exec/Process.h> to  use this */
#define Parent(T) ((struct Process*) (&PTable[T -> ParentID]))

#define TdETI(T) (T -> ExTI)
#define TdConjoint(T) (T -> ExTI.PtrConjoint)
#define TdSleepTime(T) (T -> ExTI.SleepTime)
#define TdWaiting(T) (T -> ExTI.ExecWaiters)
#define TdLock(T) (T -> Lock)

typedef VOID KTD_PARAMS;

typedef
struct {
	KTD_PARAMS *ParamList;
	SIZE ParamSize;
	VOID *ThreadEntry;
} KTHREAD_PARAMS;

typedef
struct {
	ULONG EntryMode;
	
} KTHREAD_ENTRY;

/**
 * KTHREAD - 
 *
 * Summary:
 * This type represents a schedulable kernel thread. It belongs to a process and has its
 * own set of permissions and priorities. It does not hold any 'resources'. It is just a
 * execution path, that can be run as a independent KRUNNABLE.
 *
 * Although thread belonging to the same process share its address space, threads (which
 * are allowed), can switch to another address space. This technique is used for implementing
 * controlled IPC, e.g. LPC, RPC, and Object Copying. But, the address space still remains
 * resource given to a process. 
 *
 * A thread doesn't hold any resources, but as the kernel is extendable, a hybrid type can
 * be made, between a THREAD and a PROCESS. Most user-applications don't need a special
 * entity for that, thus, it is not implemented in the KERNEL.
 *
 * NOTE:
 * KRUNNABLE interface is 
 *
 * Fields:
 * Gate - KRUNNABLE descriptor
 * ...
 *
 * @Before THREAD
 * @Version 1
 * @Since Circuit 2.03
 * @Author Shukant Pal
 */
typedef
struct _KTHREAD {
	KRUNNABLE Gate;
	UBYTE Priority;
	UBYTE Privelege;
	USHORT Flags;
	ULONG Status;
	VOID (*ProgramCounter)();
	KSTACKINFO UserStack;
	KSTACKINFO KernelStack;
	ID ParentID;//48
	ETI ExTI;
	CONTEXT *tdContext;
	SPIN_LOCK Lock;
} KTHREAD;

typedef ULONG KTHREAD_HANDLE;

extern KTHREAD *kIdlerThread;
extern KTHREAD *kInitThread;

/**
 * KeGetThread() - 
 *
 * Summary:
 * This function validates the TID and then returns the thread being referred to
 * by the TID.
 *
 * Args:
 * threadID - TID of the referred thread
 *
 * Returns: The KTHREAD structure (pointer)
 *
 * @Version 1
 * @Since Circuit 2.03
 */
KTHREAD *KeGetThread(
	ID threadID
);

VOID InitTTable(VOID);

VOID SetupRunqueue();

KTHREAD* KThreadCreate(void (*entry)());

/* Thread Operation Exceptions */
#define ParadoxError 0xE1
#define SelfDestructError 0xE2
#define HardwareException 0xE3

#endif /* Exec/Thread.h */
