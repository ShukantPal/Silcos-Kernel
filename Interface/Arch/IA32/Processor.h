/*=++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * File: Processor.h
 *
 * Summary: This file contains the interface for managing IA32-specific processor
 * functionalities and hot-plugging runqueues (and of course CPUs). Also, the
 * distro-boot feature is implemented here.
 *
 * Topological hiearchy of processors is maintained in the PROCESSOR_INFO struct. Intel has specified
 * that each processor-package/chip can have different topological levels. Thus, to manage these levels
 * individually each processor-package is assigned a package-manager CPU. This CPU constructs the
 * hiearchy tree further and manages all system-level functionalities in the package.
 *
 * Copyright (C) 2017 - Shukant Pal
 *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=*/

#ifndef X86_PROCESSOR_H
#define X86_PROCESSOR_H

#define NAMESPACE_IA32_GDT
#define NAMESPACE_IA32_IDT
#define NAMESPACE_IA32_TSS

#include "APIC.h"
#include "GDT.h"
#include "IDT.h"
#include "TSS.h"
#include <Memory/KMemorySpace.h>
#include <Util/CircularList.h>

#define CPU(ID) (PROCESSOR *) (KCPUINFO + KB(4) * ID)
#define GetProcessorById(ID) ((PROCESSOR*) (KCPUINFO + (ID << 12)))
#define PROCESSOR_ID ((U32) RdApicRegister(APIC_REGISTER_ID) >> 24)
#define PROCESSOR_STACK_SIZE 1024

VOID MxConstructTopology(
	VOID *
);

/******************************************************************************
 * Type: PROCESSOR_INFO
 *
 * Summary: This type represents the topological hierarchy of the processor and also
 * contains the platform-specific data structures.
 ******************************************************************************/
typedef
struct _PROCESSOR_INFO {
	UINT APICID;
	UINT ACPIID;
	UINT ClusterID;
	UINT PackageID;
	UINT CoreID;
	UINT SMT_ID;
	UINT ProcessorStack[256];
	GDT GDT[6];
	GDT_POINTER GDTPointer;
	TSS TSS;
	IDT IDT[256];
	IDT_POINTER IDTPointer;
	UINT TopologyIdentifiers[0];
} PROCESSOR_INFO;

typedef
struct {
	GDT DefaultBootGDT[3];
	USHORT BootStatusRegister;
	GDT_POINTER DefaultBootGDTPointer;
} PROCESSOR_SETUP_INFO __attribute__((__packed__));

#define __INTR_ON asm("sti");/* Turn interrupts on */
#define __INTR_OFF asm("cli");/* Turn interrupts off */

#ifdef ADM_DISTRO_BOOT

#endif

extern
VOID CPUID(U32 EAX, U32 ECX, U32 *registerBuffer);

/******************************************************************************
 * Function: SetupDefaultBootGDT()
 *
 * Summary: This function is used for initializing the default boot GDT, which is used in SMP
 * booting the APs, which use it for entering protected mode. It also setups the default boot
 * GDT pointer.
 *
 * @Version 1
 * @Since Circuit 2.03
 ******************************************************************************/
VOID SetupDefaultBootGDT(
	VOID
);

VOID SetupGDT(
	PROCESSOR_INFO *
);

VOID SetupTSS(
	PROCESSOR_INFO *
);

VOID SetupIDT(
	VOID
);

/******************************************************************************
 * Function: SetupBSP()
 *
 * Summary: This function is used for setting up the BSP processor. It setups the
 * early kernel-timer. It also maps its PROCESSOR struct and prepares for booting
 * other APs.
 *
 * @Version 1
 * @Since Circuit 2.03
 ******************************************************************************/
VOID SetupBSP(
	VOID
);

/******************************************************************************
 * Function: SetupProcessor()
 *
 * Summary: This function is used for mapping platform-specific structs and on
 * the x86 - IDT, GDT (non-defaultBootGDT), TSS. It then (if non-BSP processor)
 * hot-plugs a new logical runqueue.
 *
 * @Version 1
 * @Since Circuit 2.03
 ******************************************************************************/
PROCESSOR_INFO *SetupProcessor(
	VOID
);

#endif /* IA32/Processor.h */
