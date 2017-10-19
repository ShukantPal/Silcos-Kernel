/******************************************************************************
 * File: CPUID.h
 *
 * Summary: This file contains the symbols for interfacing with the CPUID feature of recent
 * CPUs.
 *
 * Functions:
 * TestCPUID() - Used for detecting if the CPUID feature is present
 * CPUID() - Used for invoking CPUID instruction
 *
 * Copyright (C) 2017 - Shukant Pal
  *****************************************************************************/
#ifndef HAL_CPUID_H
#define HAL_CPUID_H

#define CPUID_EAX 0
#define CPUID_EBX 1
#define CPUID_ECX 2
#define CPUID_EDX 3

/******************************************************************************
 * Macro Set: CPUID_VENDOR
 * 
 * Summary: These macros are given for representing the platform-vendor strings.
 *
 * @Version 1
 * @Since Circuit 2.03
 ******************************************************************************/
#define CPUID_VENDOR_OLDAMD				"AMDisbetter!"		/* Old AMD chipsets */
#define CPUID_VENDOR_AMD 					"AuthenticAMD" 	/* New AMD processors */
#define CPUID_VENDOR_INTEL 					"GenuineIntel"		/* Intel IA32/64 architectures*/
#define CPUID_VENDOR_CENTAUR				"CentaurHauls"
#define CPUID_VENDOR_OLDTRANSMETA 	"TransmetaCPU"
#define CPUID_VENDOR_TRANSMETA		"GenuineTMx86"
#define CPUID_VENDOR_CYRIX  				"CyrixInstead"
#define CPUID_VENDOR_CENTAUR 			"CentaurHauls"
#define CPUID_VENDOR_NEXGEN				"NexGenDriven"
#define CPUID_VENDOR_UMC 					"UMC UMC UMC "
#define CPUID_VENDOR_SIS 						"SiS SiS SiS "
#define CPUID_VENDOR_NSC 					"Geode by NSC"
#define CPUID_VENDOR_RISE 					"RiseRiseRise"
#define CPUID_VENDOR_VORTEX 				"Vortex86 SoC"
#define CPUID_VENDOR_VIA 						"VIA VIA VIA "
#define CPUID_VENDOR_VMWARE 			"VMwareVMware"
#define CPUID_VENDOR_XENHVM 				"XenVMMXenVMM"
#define CPUID_VENDOR_MICROSOFT_HV 	"Microsoft Hv"
#define CPUID_VENDOR_PARALLELS 			" lrpepyh vr"

/******************************************************************************
 * Enum: CPUID_FEATURE
 *
 * Summary: 
 *
 * @Version 1
 * @Since Circuit 2.03
 ******************************************************************************/
typedef
enum {
 	CPUID_FEAT_ECX_SSE3 = 1 << 0, 
 	CPUID_FEAT_ECX_PCLMUL= 1 << 1,
 	CPUID_FEAT_ECX_DTES64 = 1 << 2,
 	CPUID_FEAT_ECX_MONITOR= 1 << 3,  
	CPUID_FEAT_ECX_DS_CPL = 1 << 4,  
	CPUID_FEAT_ECX_VMX= 1 << 5,  
	CPUID_FEAT_ECX_SMX = 1 << 6,  
 	CPUID_FEAT_ECX_EST = 1 << 7,  
 	CPUID_FEAT_ECX_TM2 = 1 << 8,  
 	CPUID_FEAT_ECX_SSSE3= 1 << 9,  
 	CPUID_FEAT_ECX_CID = 1 << 10,
 	CPUID_FEAT_ECX_FMA = 1 << 12,
 	CPUID_FEAT_ECX_CX16 = 1 << 13, 
 	CPUID_FEAT_ECX_ETPRD= 1 << 14, 
	CPUID_FEAT_ECX_PDCM = 1 << 15, 
 	CPUID_FEAT_ECX_DCA= 1 << 18, 
 	CPUID_FEAT_ECX_SSE4_1 = 1 << 19, 
 	CPUID_FEAT_ECX_SSE4_2 = 1 << 20, 
 	CPUID_FEAT_ECX_x2APIC = 1 << 21, 
 	CPUID_FEAT_ECX_MOVBE= 1 << 22, 
 	CPUID_FEAT_ECX_POPCNT = 1 << 23, 
 	CPUID_FEAT_ECX_AES = 1 << 25, 
 	CPUID_FEAT_ECX_XSAVE = 1 << 26, 
 	CPUID_FEAT_ECX_OSXSAVE = 1 << 27, 
 
 	CPUID_FEAT_EDX_FPU  = 1 << 0,  
	CPUID_FEAT_EDX_VME  = 1 << 1,  
 	CPUID_FEAT_EDX_DE  = 1 << 2,  
 	CPUID_FEAT_EDX_PSE  = 1 << 3,  
 	CPUID_FEAT_EDX_TSC  = 1 << 4,  
 	CPUID_FEAT_EDX_MSR  = 1 << 5,  
 	CPUID_FEAT_EDX_PAE  = 1 << 6,  
	CPUID_FEAT_EDX_MCE  = 1 << 7,  
	CPUID_FEAT_EDX_CX8  = 1 << 8,  
	CPUID_FEAT_EDX_APIC = 1 << 9,  
	CPUID_FEAT_EDX_SEP  = 1 << 11, 
	CPUID_FEAT_EDX_MTRR = 1 << 12, 
	CPUID_FEAT_EDX_PGE  = 1 << 13, 
	CPUID_FEAT_EDX_MCA  = 1 << 14, 
	CPUID_FEAT_EDX_CMOV = 1 << 15, 
	CPUID_FEAT_EDX_PAT  = 1 << 16, 
	CPUID_FEAT_EDX_PSE36= 1 << 17, 
	CPUID_FEAT_EDX_PSN  = 1 << 18, 
	CPUID_FEAT_EDX_CLF  = 1 << 19, 
	CPUID_FEAT_EDX_DTES = 1 << 21, 
	CPUID_FEAT_EDX_ACPI = 1 << 22, 
	CPUID_FEAT_EDX_MMX  = 1 << 23, 
 	CPUID_FEAT_EDX_FXSR = 1 << 24, 
	CPUID_FEAT_EDX_SSE  = 1 << 25, 
	CPUID_FEAT_EDX_SSE2 = 1 << 26, 
	CPUID_FEAT_EDX_SS= 1 << 27,
	CPUID_FEAT_EDX_HTT  = 1 << 28, 
	CPUID_FEAT_EDX_TM1  = 1 << 29, 
	CPUID_FEAT_EDX_IA64 = 1 << 30,
	CPUID_FEAT_EDX_PBE  = 1 << 31
} CPUID_FEATURE;

/******************************************************************************
 * Enum: CPU_REQUEST
 *
 * Summary:
 * 
 * @Version 1
 * @Since Circuit 2.03
 ******************************************************************************/
enum {
	CPUID_GETVENDORSTRING = 0x0,
 	CPUID_GETFEATURES =0x1,
  	CPUID_GETTLB = 0x2,
  	CPUID_GETSERIALNO = 0x3,
 
  	CPUID_INTELEXTENDED=0x80000000,
  	CPUID_INTELFEATURES,
  	CPUID_INTELBRANDSTRING,
  	CPUID_INTELBRANDSTRINGMORE,
  	CPUID_INTELBRANDSTRINGEND,
} CPUID_REQUEST;

/******************************************************************************
 * Function: TestCPUID()
 *
 * Summary: This function tests whether CPUID is supported on the given platform, and
 * returns a non-zero value on success.
 *
 * Return: Non-zero value on success
 *
 * @Version 1
 * @Since Circuit 2.03
 ******************************************************************************/
extern
ULONG TestCPUID(
	VOID
);

/******************************************************************************
 * Function: CPUID()
 *
 * Summary: This function invokes the CPUID on the concerned architecuture and returns
 * the result in the memory buffer given. This memory buffer must coincide with the needs
 * of the specific platform.
 *
 * Args:
 * ULONG EAX - No. to put in EAX before invocation
 * ULONG ECX - No. to put in ECX before invocation
 * ULONG *destinationPointer - Memory buffer to load results
 *
 * @Version 1
 * @Since Circuit 2.03
 ******************************************************************************/
extern
VOID CPUID(
	U32 EAX,
	U32 ECX,
	U32 *destinationPointer
);

#endif/* HAL/CPUID.h */
