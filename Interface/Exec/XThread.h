/**
 * Copyright (C) 2017 - Shukant Pal
 *
 * Here, we present (kernel-side of) phase-I of the SCI. Thread-manipulative
 * functions are declared and can be used (without) system-calling.
 */
#ifndef EXEC_X_THREAD_H
#define EXEC_X_THREAD_H

#include "Thread.h"
#include <HAL/Processor.h>
#include "XTP.h"

/* System Call Interface - Phase I (Threading) */

/* NOTE : XTID be implemented in assembly. */

/* Create Thread */
ULONG XCreateThread(
	KTHREAD *Caller,
	VOID (*Entry) (),
	RUNTIME_INFO Runtime,
	ULONG StackBase // If CustomStack Flag is set.
);

VOID KiExitThread(
	ULONG exitStatus
);

/**
 * KeExitThread() - 
 *
 * Summary:
 * This function destroys the current thread (without looking into security),
 * and returns the exit status to all other threads waiting on this one. It frees
 * all dependencies on this thread.
 *
 * Args:
 * callerThread - The thread exiting
 * exitStatus - Exit status
 *
 * Returns: VOID
 *
 * @Version 1
 * @Since Circuit 2.03
 */
VOID XExitThread(
	KTHREAD *callerThread,
	ULONG exitStatus
);

/* Wait upon thread for exiting. */
VOID XWaitForThread(
	KTHREAD *Caller,
	ID TID
);

/* Sleep for a number of kernel ticks. */
VOID XSleepThread(
	KTHREAD *Caller,
	TIME SleepTime // Millisecond 
);

/* Wait for thread for a period of time. */
VOID XWaitForThreadTill(
	KTHREAD *Caller,
	ID TID,
	TIME SleepTime
);

/* Kill a thread */
ULONG XDeleteThread(
	KTHREAD *Caller,
	ID TID
);

/* Pause a thread for some time (or until manually woken) */
VOID XStopThread(
	KTHREAD *Caller,
	ID TID,
	TIME StopTime
);

/* Waken a stopped or sleeping thread. */
VOID XWakeupThread(
	KTHREAD *Caller,
	ID TID
);

#endif /* Exec/XThread.h */
