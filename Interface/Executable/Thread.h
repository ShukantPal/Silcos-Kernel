/**
 * Copyright (C) 2017 - Shukant Pal
 */
#ifndef EXECUTABLE_THREAD_H__
#define EXECUTABLE_THREAD_H__

#include <Executable/CPUStack.h>
#include <Executable/Task.hpp>
#include <Memory/Pager.h>
#include <Synch/Spinlock.h>
#include <Types.h>

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

#define UserThread 1
#define KernelThread 0

/* Thread in user-mode or in kernel-mode. */
#define ThreadInUser(T) ((TFlags(T) >> 1) & 1)
#define ThreadInKernel(T) (!ThreadInUser(T))

#define CustomStack(T) (TdFlags(T) >> 4 & 1)

struct Thread
{
	Executable::Task Gate;
	UBYTE Priority;
	UBYTE Privelege;
	unsigned short Flags;
	unsigned long Status;
	void (*ProgramCounter)();
	CPUStack UserStack;
	CPUStack KernelStack;
	ID ParentID;//48
	CONTEXT *tdContext;
	Spinlock Lock;
};

typedef unsigned long KTHREAD_HANDLE;

extern Thread *kIdlerThread;
extern Thread *kInitThread;

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
 * Returns: The Thread structure (pointer)
 *
 * @Version 1
 * @Since Circuit 2.03
 */
Thread *KeGetThread(ID threadID);

void InitTTable(void);
void SetupRunqueue();
Thread* KThreadCreate(void *entry);

#endif/* Executable/Thread.h */
