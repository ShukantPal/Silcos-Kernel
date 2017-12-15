/**
 * File: Processor.h
 *
 * Summary:
 * Declares how the IA32-processor-specific data is maintained in each per-CPU
 * struct. Also defines some initialization specific code used during SMP-wake
 * to initialize application-processors.
 *
 * Copyright (C) 2017 - Shukant Pal
 */
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

void MxConstructTopology(void *);

typedef
struct ProcessorInfo {
	unsigned int APICID;
	unsigned int ACPIID;
	unsigned int ClusterID;
	unsigned int PackageID;
	unsigned int CoreID;
	unsigned int SMT_ID;
	unsigned int ProcessorStack[256];
	GDTEntry GDT[6];
	GDTPointer GDTR;
	TSS kTSS;
	IDTEntry IDT[256];
	IDTPointer IDTR;
	unsigned int TopologyIdentifiers[0];
} PROCESSOR_INFO;

typedef
struct {
	GDTEntry DefaultBootGDT[3];
	unsigned short BootStatusRegister;
	GDTPointer DefaultBootGDTPointer;
} PROCESSOR_SETUP_INFO;

#define __INTR_ON asm("sti");
#define __INTR_OFF asm("cli");

extern "C"
{
void SetupDefaultBootGDT(void);
void SetupGDT(ProcessorInfo *);
void SetupTSS(ProcessorInfo *);
void SetupIDT(void);
void SetupBSP(void);
void SetupAPs(void);
ProcessorInfo *SetupProcessor(void);
}

#endif/* IA32/Processor.h */
