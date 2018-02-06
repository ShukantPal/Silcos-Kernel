/**
 * File: Processor.h
 *
 * Summary:
 * This file presents the interface for managing multi-processing in the kernel. Each CPU must be
 * handled seperately by the software and this is achieved by keeping information of each CPU in
 * table. Each CPU is given 4 (32-bit) or 8 (64-bit) KB for keeping its information and this is done in
 * the Processor struct. Each processor is identified by its PROCESSOR_ID (platform-specific) and
 * this is the index for it in the processor table which is placed at KCPUINFO in the kernel memory. 
 *
 * Processor topology is also maintained by the system in order to optimize resource allocation. Each
 * processor is kept in a system-topology tree which is maintained in a platform-specific manner. A group
 * of processors is referred to by SCHED_GROUP and an area of scheduling is called SCHED_DOMAIN. Here,
 * SCHED_GROUPs are the children of a SCHED_DOMAIN.
 *
 * Types:
 * Processor - per-CPU data struct
 * PROCESSOR_TOPOLOGY - used for managing processor topology and sched domains
 *
 * Functions:
 * AddProcessorInfo() - Setup Processor struct
 * SetupProcessor() - HAL-init per-CPU
 * MxRegisterProcessor() - Register processor topology in the system
 * MxIterateTopology() - Do some operation on each domain in which this processor exists
 *
 * Copyright (C) 2017 - Shukant Pal
 */
#ifndef HAL_PROCESSOR_H__
#define HAL_PROCESSOR_H__

#include <IA32/APIC.h>
#include "ProcessorTopology.hpp"
#include <ACPI/MADT.h>
#include <Executable/RoundRobin.h>
#include <Memory/CacheRegister.h>
#include <Memory/KMemorySpace.h>
#include <Synch/Spinlock.h>
#include "../Utils/AVLTree.hpp"
#include "../Utils/AVLTree.hpp"
#include "../Utils/CircularList.h"
#include "../Utils/LinkedList.h"
#include "../Utils/Memory.h"

#ifdef x86
	#include <IA32/Processor.h>
#endif

namespace Executable
{
	class ScheduleRoller;
	class RoundRobin;
}

extern U32 BSP_HID;

// legacy
#define PROCESSOR_HIEARCHY_SYSTEM 				0xFFFFFFFF
#define PROCESSOR_HIEARCHY_CLUSTER				10
#define PROCESSOR_HIEARCHY_PACKAGE				
#define PROCESSOR_HIERARCHY_LOGICAL_CPU 1

/* Helper macros */
#define PoID(P) (P -> Hardware)
#define HardwareHeader(P) (P -> Hardware)
#define SID(P) (P -> SId)

typedef
struct ScheduleInfo
{
	unsigned long Load;
	unsigned long RunnerPopulation;
	Executable::ScheduleRoller *presRoll;// presently running schedule-roller
	unsigned long CurrentQuanta;// quanta given to current runner
	unsigned long RunnerInterruptable;// if task is pre-emptible
	unsigned long LeftQuanta;// quanta left-over
	unsigned long FlagSet;// runtime flags
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

namespace HAL
{

/**
 * Enum: RequestType
 *
 * Summary:
 * When a processor recieves an IPI on the 254th vector then the act-on-request
 * handler executes which takes the following input paramaters (on the
 * actionRequests list).
 *
 * Author: Shukant Pal
 */
enum RequestType
{
	/*
	 * When a overloaded processor transfers tasks to another processor,
	 * then it will send a 'AcceptTask' request, which is handled by
	 * adding the listed tasks into the recieving cpu's runqueue.
	 *
	 * It uses the RunqueueBalancer::Accept struct (allocated by
	 * tRunqueueBalancer_Accept)
	 */
	AcceptTasks,

	/*
	 * When a underloaded processor asks for tasks to another processor,
	 * it will send a 'RenounceTask' request, which is handled by
	 * sending back a 'AcceptTask' request which contains a list of
	 * transferrable tasks.
	 *
	 * It uses the RunqueueBalanacer::Renounce struct (allocated by
	 * tRunqueueBalancer_Renounce)
	 */
	RenounceTasks
};

/**
 * Struct: ProcessorBinding::IPIRequest
 *
 * Summary:
 * This is the parent-class for all inter-processor requests sent over
 * the Processor::actionRequests list.
 *
 * Variables:
 * type - request-type sent
 * bufferSize - size of a dynamic-memory (allocated with kmalloc) buffer
 * source - slab-allocator for the type (used for freeing)
 *
 * Author: Shukant Pal
 */
struct IPIRequest
{
	CircularListNode reqlink;
	const RequestType type;
	const unsigned long bufferSize;// size of memory-buffer, if any used
	const ObjectInfo *source;

	IPIRequest(RequestType mtype, unsigned long bsize, ObjectInfo *src)
			: type(mtype), bufferSize(bsize), source(src)
	{
	}
};

struct Processor
{
	LinkedListNode LiLinker;/* Participate in lists */
	unsigned int ProcessorCluster;/* NUMA Domain */
	unsigned short PoLoad;
	unsigned char ProcessorStatus;
	unsigned char PoFreq;
	unsigned int PoType;
	unsigned short PoStk;
	unsigned short Padding;
	Executable::Task *ctask;// presently running task on this cpu
	unsigned int ProcessorStack;// address of the unique processor-stack
	ScheduleInfo crolStatus;// core-roller (scheduler) status
	CHREG frameCache[5];
	CHREG pageCache[2];
	CHREG slabCache;
	Spinlock PageLock;/* Obselete */
	Executable::ScheduleRoller *lschedTable[3];// ptr-table to schedule-classes
	Executable::RoundRobin rrsched;// round-robin scheduler
	void *IdlerThread;// idle-task for this cpu
	void *SetupThread;// initialization thread
	HAL::Domain *domlink;// link into the topology tree
	CircularList actionRequests;// list of action-request waiting for handling
	Spinlock migrlock;// migration lock (for registering action-requests)
	AVLTree timeoutTree;// tree containing tasks that are waiting on the timer
	ArchCpu hw;// platform-specifics
};

extern Processor *CPUArray;

class CPUDriver
{
public:
	static IPIRequest *readRequest(Processor *proc);
	static void writeRequest(IPIRequest& state, Processor *proc);
};

}// namespace HAL

decl_c void AddProcessorInfo(MADTEntryLAPIC *madtEntry);

#endif/* HAL/Processor.h */
