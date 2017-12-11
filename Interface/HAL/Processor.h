/**
 * File: Processor.h
 *
 * Summary:
 * This file presents the interface for managing multi-processing in the kernel. Each CPU must be
 * handled seperately by the software and this is achieved by keeping information of each CPU in
 * table. Each CPU is given 4 (32-bit) or 8 (64-bit) KB for keeping its information and this is done in
 * the PROCESSOR struct. Each processor is identified by its PROCESSOR_ID (platform-specific) and
 * this is the index for it in the processor table which is placed at KCPUINFO in the kernel memory. 
 *
 * Processor topology is also maintained by the system in order to optimize resource allocation. Each
 * processor is kept in a system-topology tree which is maintained in a platform-specific manner. A group
 * of processors is referred to by SCHED_GROUP and an area of scheduling is called SCHED_DOMAIN. Here,
 * SCHED_GROUPs are the children of a SCHED_DOMAIN.
 *
 * Types:
 * PROCESSOR - per-CPU data struct
 * PROCESSOR_TOPOLOGY - used for managing processor topology and sched domains
 *
 * Functions:
 * AddProcessorInfo() - Setup PROCESSOR struct
 * SetupProcessor() - HAL-init per-CPU
 * MxRegisterProcessor() - Register processor topology in the system
 * MxIterateTopology() - Do some operation on each domain in which this processor exists
 *
 * Copyright (C) 2017 - Shukant Pal
 */
#ifndef HAL_PROCESSOR_H
#define HAL_PROCESSOR_H

#include "ProcessorTopology.hpp"
#include <ACPI/MADT.h>
#include <Exec/CFS.h>
#include <Exec/RR.h>
#include <Memory/CacheRegister.h>
#include <Memory/KMemorySpace.h>
#include <Exec/SchedList.h>
#include <Util/AVLTree.h>
#include <Util/CircularList.h>
#include <Util/LinkedList.h>
#include <Util/Memory.h>
#include <Synch/Spinlock.h>

#ifdef x86
	#include <IA32/Processor.h>
#endif

extern U32 BSP_HID;

#define PROCESSOR_HIEARCHY_SYSTEM 				0xFFFFFFFF
#define PROCESSOR_HIEARCHY_CLUSTER				10
#define PROCESSOR_HIEARCHY_PACKAGE				
#define PROCESSOR_HIERARCHY_LOGICAL_CPU 1
/*
typedef
struct _PROCESSOR_TOPOLOGY
{
	union {
		LINODE LiLinker;///* Used for participating in lists *
		CLNODE ClnLinker;
		struct {
			struct _PROCESSOR_TOPOLOGY *NextDomain;
			struct _PROCESSOR_TOPOLOGY *PreviousDomain;
		};
	};
	UINT DomainID;///* ID for this group *
	UINT Level;///* Topology Level (inside processor package) *
	UINT Type;///* Topology Type (outside the package) *
	UINT ProcessorCount;///* No. of processors in the domain *
	KSCHED_ROLLER_DOMAIN SchedDomain[3];///* Scheduler Domain Info *
	CLIST DomainList;///* (Sorted) List of SCHED_GROUPs *
	struct _PROCESSOR_TOPOLOGY *ParentDomain;///* Parent SCHED_DOMAIN *
	SPIN_LOCK WriteLock;///* Lock for manipulating stats *
} PROCESSOR_TOPOLOGY;
*/
struct _PROCESSOR;
typedef _PROCESSOR Processor;


//typedef PROCESSOR_TOPOLOGY SCHED_DOMAIN;/* Scheduling Domain */
//typedef PROCESSOR_TOPOLOGY SCHED_GROUP;/* Scheduable Groups inside a domain */
//typedef PROCESSOR_TOPOLOGY PROCESSOR_GROUP;/* Generic #TERM */

#define MXRS_PROCESSOR_ALREADY_EXISTS	0xFAE0
#define MXRS_PROCESSOR_STRUCT_INVALID	0xFAE1
#define MXRS_PROCESSOR_REGISTERED		0xFAEF
typedef ULONG MX_REGISTER_STATUS;

/**
 * enum PoS - 
 *
 * This state tells about the execution
 * condition of the PoC. 
 *
 * ~ PoSRunning - Executing a valid thread
 * ~ PoSHalt - Not running or sleeping
 * ~ PoSInit - Initializing
 *
 */
enum PoS
{
	PoSRunning = 0x1,
	PoSHalt = 0x2,
	PoSInit = 0x3
};

/**
 *
 * Tells about specific state of a PoC global
 * stack.
 *
 * The scheduler & syscall handlers use these
 * stacks to execute.
 *
 */
enum
{
	PoStkNULL = 0x1,
	PoStkUSED = 0x2,
	PoStkEXP = 0x3
};

/* Helper macros */
#define PoID(P) (P -> Hardware)
#define HardwareHeader(P) (P -> Hardware)
#define SID(P) (P -> SId)

/* Use only while including Thread.h & Process.h */
#define ExecThread_ptr(P) (P -> PoExT)

#define ExecThread_(P) ((struct Thread *) P -> PoExT)
#define ExecProcess_(P) ((struct Process *) &PTable[ExecThread_(P) -> ParentID])

#define ExecThread(P) ((struct Thread *) P -> PoExT)
#define ExecProcess(P) ((struct Process *) &PTable[ExecThread(P) -> ParentID])

typedef
struct _KSCHEDINFO {
	ULONG Load;
	ULONG RunnerPopulation;
	KSCHED_ROLLER *CurrentRoller;/* Current task's scheduler */
	ULONG CurrentQuanta;/* Quanta given to current runner */
	ULONG RunnerInterruptable;/* Is pre-emption allowed */
	ULONG LeftQuanta;/* Amount of quanta left-over */
	ULONG FlagSet;/* Runtime-FLAGS */
	ExecList SchedQueue;/* Queue immediate look */
} KSCHEDINFO;

#ifdef NAMESPACE_MEMORY_MANAGER
	#define __PgInitOp__(CPU) SpinLock(&CPU->PageLock);
	#define __PgExitOp__(CPU) SpinUnlock(&CPU->PageLock);
#endif

#ifdef x86
	#define FRCH_OFFSET 32 + sizeof(KSCHEDINFO)
	#define PGCH_OFFSET FRCH_OFFSET + 5 * sizeof(CHREG)
	#define SLBCH_OFFSET SLBCH_OFFSET + 2 * sizeof(CHREG)
#endif

typedef
struct _PROCESSOR {
	LINODE LiLinker;/* Participate in lists */
	UINT ProcessorCluster;/* NUMA Domain */
	USHORT PoLoad;
	UCHAR ProcessorStatus;
	UCHAR PoFreq;
	UINT PoType;
	USHORT PoStk;
	USHORT Padding;
	VOID *PoExT;/* Current Runner */
	UINT ProcessorStack;/* UPKS */
	KSCHEDINFO SchedulerInfo;/* Scheduler Status */
	CHREG FrameCache[5];/* KFrameManager::CacheRegister */
	CHREG PageCache[2];/* KMemoryManager::CacheRegister */
	CHREG SlabCache;/* KObjectManager::CacheRegister */
	SPIN_LOCK PageLock;/* Obselete */
	KSCHED_ROLLER ScheduleClasses[3];/* Scheduler Classes */
	SCHED_CFS CFS;/* CFS-Scheduler Status */
	SCHED_RR RR;/* RR-Scheduler Status */
	void *IdlerThread;/* Task for idle CPU */
	void *SetupThread;/* Maintenance and setup task */
	HAL::Domain *DomainInfo;/* Processor Topology & Scheduling Domains */
	PROCESSOR_INFO Hardware;/* Platform-specific data */
} PROCESSOR;

extern PROCESSOR *CPUArray;

void SendIPI(APIC_ID apicId, APIC_VECTOR vectorIndex);
void AddProcessorInfo(MADTEntryLAPIC *madtEntry);
void SendIPI(APIC_ID apicId, APIC_VECTOR vectorIndex);

#endif /* HAL/Processor.h */
