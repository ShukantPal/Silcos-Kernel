/**
 * @file Processor.hpp
 * @module HAL (kernel.silcos.hal)
 *
 * Holds declarations related to <tt>Processor</tt> object related
 * functions. Provides a driver to access <b>symmetric multiprocessing
 * </b> function (note that asymmetric functionality will be implemented
 * as a separate interface).
 * -------------------------------------------------------------------
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 *
 * Copyright (C) 2017 - Shukant Pal
 */
#ifndef HAL_PROCESSOR_H__
#define HAL_PROCESSOR_H__

#include <IA32/APIC.h>
#include <ACPI/MADT.h>
#include <Executable/RoundRobin.h>
#include <HardwareAbstraction/ProcessorTopology.hpp>
#include <Memory/Internal/CacheRegister.h>
#include <Memory/KMemorySpace.h>
#include <Synch/Spinlock.h>
#include <Utils/AVLTree.hpp>
#include <Utils/CircularList.h>
#include <Utils/LinkedList.h>
#include <Utils/Memory.h>

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
#define PROCESSOR_HIEARCHY_SYSTEM 		0xFFFFFFFF
#define PROCESSOR_HIEARCHY_CLUSTER		10
#define PROCESSOR_HIEARCHY_PACKAGE				
#define PROCESSOR_HIERARCHY_LOGICAL_CPU		1

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
 * When a processor recieves an IPI on the 254th vector then the act-on-request
 * handler executes which takes the following input paramaters (on the
 * actionRequests list).
 *
 * @author Shukant Pal
 */
enum RequestType
{
	ACCEPT_TASK_COLLECTION,
	RENOUNCE_TASK_COLLECTION,
	INVOKE_OPERATION,
	INVOKE_OPERATION_THIS
};

/**
 * Holds a packaged request sent between two CPUs, holding the operation type
 * and (an optional) callback. These object can be used using <tt>CPUDriver</tt>
 * which handles the sending and receiving of this structures.
 *
 * <tt>IPIRequest</tt> can be extended, but doing so, the proper <tt>source</tt>
 * allocator must be specified in-order to clean the object successfully. Use
 * the <tt>INVOKE_CALLBACK_THIS</tt> action to pass the extended structure
 * to the callback
 * 
 * @version 1.0
 * @since Circuit 2.03
 * @author Shukant Pal
 */
struct IPIRequest
{	
	CircularListNode reqlink;
	const RequestType type;
	const unsigned long bufferSize;// size of memory-buffer, if any used
	const ObjectInfo *source;
	
	union {
		void (*callbackDefault)();
		void (*callbackThis)(IPIRequest *extendedObject);
	};

	IPIRequest(RequestType mtype, unsigned long bsize, ObjectInfo *src)
			: type(mtype), bufferSize(bsize), source(src) {
	}
};

/**
 * Holds processor-specific entries that are cache-local. Each <tt>Processor
 * </tt> is accessed only by the owner chip, although this is not enforced
 * proper in the code. These objects are always aligned on a 32-KB boundary
 * in alloted permanent memory.
 *
 * Each <tt>Processor</tt> object registered in the kernel, can be accessed
 * by calling <tt>GetProcessorById(PROCESSOR_ID)</tt>.
 *
 * <tt>Processor</tt> objects mostly contain subsystem-specific data, which
 * is not documented as a part of the <tt>Processor</tt> struct.
 *
 * @version 10.2
 * @since Circuit 2.01
 * @author Shukant Pal
 */
struct Processor
{
	LinkedListNode LiLinker;
	unsigned int ProcessorCluster;//! @deprecated
	unsigned short PoLoad;//! @deprecated
	unsigned char ProcessorStatus;//! @deprecated
	unsigned char PoFreq;//! frequency of the cpu (not used yet!)
	unsigned int PoType;//! @deprecated
	unsigned short PoStk;//! @deprecated
	unsigned short Padding;//! padding for 16-bit number
	Executable::Task *ctask;//! currently running task
	unsigned int ProcessorStack;//! address of per-cpu stack
	ScheduleInfo crolStatus;//! scheduling status
	CHREG frameCache[5];//! cache for page-frames
	CHREG pageCache[2];//! cache for kernel-memory pages
	CHREG slabCache;//! cache for slabs
	Spinlock PageLock;
	Executable::ScheduleRoller *lschedTable[3];//! table for sched-classes
	Executable::RoundRobin rrsched;//! round-robin scheduler state
	void *IdlerThread;//! idle-task for this cpu
	void *SetupThread;//! initialization thread for this cpu
	HAL::Domain *domlink;//! link to topology-tree
	CircularList actionRequests;//! group of ipi-requests pending
	Spinlock migrlock;//! migration lock for tasks
	AVLTree timeoutTree;//! contains tasks sleeping until a specific time
	ArchCpu hw;//!< This contains information about the CPU which directly
	 	   //!< directly depends on the platform. @see IA32/Processor.h
};

extern Processor *CPUArray;

/**
 * Provides a driver-like interface to access low-level SMP logic. Kernel
 * subsystems can use this to execute specific, low-latency jobs on another
 * processor.
 *
 * @version 1.0
 * @since Silcos 2.05
 * @author Shukant Pal
 */
class CPUDriver
{
public:
	static IPIRequest *readRequest(Processor *proc);
	static void writeRequest(IPIRequest& state, Processor *proc);
};

}// namespace HAL

decl_c void AddProcessorInfo(MADTEntryLAPIC *madtEntry);

#endif/* HAL/Processor.h */
