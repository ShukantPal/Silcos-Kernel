/**
 * Copyright (C) - Shukant Pal
 *
 * A basic interface for thread sub-scheduling is provided here.
 */
#ifndef EXEC_THREAD_GROUP_H
#define EXEC_THREAD_GROUP_H

#include <Executable/KTask.h>
#include <Executable/SchedList.h>

/* Helper macro(s) */
#define TGChildren(TG) &(TG -> Children)

/**
 * KTHREAD_GROUP - 
 *
 * Summary:
 * It represents a collection of (kernel) threads, from which only one should be
 * scheduled at once (fairly). It is a single entity for the scheduler but contains
 * more scheduling units, and has a sub-scheduler.
 *
 * @Version 1
 * @Since Circuit 2.01
 * @Author Shukant Pal
 */
typedef
struct ThreadGroup
{
	struct KTask Gate;
	ExecList Children;
} KTHREAD_GROUP;

#endif /* Exec/ThreadGroup.h */
