/**
 * File: CPUID.h
 *
 * Summary:
 * Provides the required abstraction
 *
 * Functions:
 * TestCPUID - Used for detecting if the CPUID feature is present
 * CPUID - Used for invoking CPUID instruction
 *
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
#ifndef HAL_CPUID_H
#define HAL_CPUID_H

#include <TYPE.h>

namespace HAL
{
namespace CpuId
{

union cRegs
{
	U32 output[4];
	struct
	{
		U32 eax;
		U32 ebx;
		U32 ecx;
		U32 edx;
	};
};

#define CV_OLD_AMD			"AMDisbetter!"
#define CV_AMD				"AuthenticAMD"
#define CV_INTEL			"GenuineIntel"
#define CV_CENTAUR			"CentaurHauls"
#define CV_OLD_TRANSMETA	"TransmetaCPU"
#define CV_TRANSMETA		"GenuineTMx86"
#define CV_CYRIX			"CyrixInstead"
#define CV_NEXGEN			"NexGenDriven"
#define CV_UMC				"UMC UMC UMC "
#define CV_SIS				"SiS SiS SiS "
#define CV_NSC				"Geode by NSC"
#define CV_RISE				"RiseRiseRise"
#define CV_VORTEX			"Vortex64 SoC"
#define CV_VIA				"VIA VIA VIA "
#define CV_VMWARE			"VMWareVMWare"
#define CV_XEN				"XenVMMXenWMM"
#define CV_MICROSOFT_HV		"Microsoft HV"
#define CV_PARALLELS		" lrpepyh vr"

enum Feature
{
 	CPUID_FEAT_ECX_SSE3	= 1 << 0,
 	CPUID_FEAT_ECX_PCLMUL	= 1 << 1,
 	CPUID_FEAT_ECX_DTES64	= 1 << 2,
 	CPUID_FEAT_ECX_MONITOR	= 1 << 3,
	CPUID_FEAT_ECX_DS_CPL	= 1 << 4,
	CPUID_FEAT_ECX_VMX	= 1 << 5,
	CPUID_FEAT_ECX_SMX	= 1 << 6,
 	CPUID_FEAT_ECX_EST	= 1 << 7,
 	CPUID_FEAT_ECX_TM2	= 1 << 8,
 	CPUID_FEAT_ECX_SSSE3	= 1 << 9,
 	CPUID_FEAT_ECX_CID	= 1 << 10,
 	CPUID_FEAT_ECX_FMA	= 1 << 12,
 	CPUID_FEAT_ECX_CX16	= 1 << 13,
 	CPUID_FEAT_ECX_ETPRD	= 1 << 14,
	CPUID_FEAT_ECX_PDCM	= 1 << 15,
 	CPUID_FEAT_ECX_DCA	= 1 << 18,
 	CPUID_FEAT_ECX_SSE4_1	= 1 << 19,
 	CPUID_FEAT_ECX_SSE4_2	= 1 << 20,
 	CPUID_FEAT_ECX_x2APIC	= 1 << 21,
 	CPUID_FEAT_ECX_MOVBE	= 1 << 22,
 	CPUID_FEAT_ECX_POPCNT	= 1 << 23,
 	CPUID_FEAT_ECX_AES	= 1 << 25,
 	CPUID_FEAT_ECX_XSAVE	= 1 << 26,
 	CPUID_FEAT_ECX_OSXSAVE	= 1 << 27,
 
 	CPUID_FEAT_EDX_FPU	= 1 << 0,
	CPUID_FEAT_EDX_VME	= 1 << 1,
 	CPUID_FEAT_EDX_DE	= 1 << 2,
 	CPUID_FEAT_EDX_PSE	= 1 << 3,
 	CPUID_FEAT_EDX_TSC	= 1 << 4,
 	CPUID_FEAT_EDX_MSR	= 1 << 5,
 	CPUID_FEAT_EDX_PAE	= 1 << 6,
	CPUID_FEAT_EDX_MCC	= 1 << 7,
	CPUID_FEAT_EDX_CX8	= 1 << 8,
	CPUID_FEAT_EDX_APIC	= 1 << 9,
	CPUID_FEAT_EDX_SEP	= 1 << 11,
	CPUID_FEAT_EDX_MTRR	= 1 << 12,
	CPUID_FEAT_EDX_PGE	= 1 << 13,
	CPUID_FEAT_EDX_MCA	= 1 << 14,
	CPUID_FEAT_EDX_CMOV	= 1 << 15,
	CPUID_FEAT_EDX_PAT	= 1 << 16,
	CPUID_FEAT_EDX_PSE36	= 1 << 17,
	CPUID_FEAT_EDX_PSN	= 1 << 18,
	CPUID_FEAT_EDX_CLF	= 1 << 19,
	CPUID_FEAT_EDX_DTES	= 1 << 21,
	CPUID_FEAT_EDX_ACPI	= 1 << 22,
	CPUID_FEAT_EDX_MMX	= 1 << 23,
 	CPUID_FEAT_EDX_FXSR	= 1 << 24,
	CPUID_FEAT_EDX_SSE	= 1 << 25,
	CPUID_FEAT_EDX_SSE2	= 1 << 26,
	CPUID_FEAT_EDX_SS	= 1 << 27,
	CPUID_FEAT_EDX_HTT	= 1 << 28,
	CPUID_FEAT_EDX_TM1	= 1 << 29,
	CPUID_FEAT_EDX_IA64	= 1 << 30,
	CPUID_FEAT_EDX_PBE	= 1 << 31
};

enum Leaf
{
	VendorString			= 0x0,
	Features				= 0x1,
	TLBInfo					= 0x2,
	PSN						= 0x3,
	ExtendedFeatures		= 0x7,
	DirectCacheAccessInfo	= 0x9,
	ArchPerfMonitoring		= 0xA,
	ExtendedTopologyEnum	= 0xB,
	ExtendedStateEnum		= 0xD,
	TimeStampCounter		= 0x15,
	FrequencyInfo			= 0x16,
	SoCVendorAttributeEnum	= 0x17,
	DetermAddrTransParam	= 0x18,
	ExtendedFuncCPUIDInfo	= 0x80000000,
	SignAndFeatureBits		= 0x80000001,
	BrandStringStart		= 0x80000002,
	BrandStringMiddle		= 0x80000003,
	BrandStringEnd			= 0x80000004,
	InvariantTSC			= 0x80000007,
	LinearAndPhysAddrSize	= 0x80000008
};

namespace Intel
{
enum ProcessorType
{
	OrgOEM		= 0b00,
	OverDrive	= 0b01,
	DualProc	= 0b10,
	Reserved	= 0b11
};

struct VersionInformation
{
	U32 steppingID	: 4;
	U32 model		: 4;
	U32 familyID	: 4;
	U32 cpuType		: 2;
	U32 rField0		: 2;
	U32 extModelID	: 4;
	U32 extFamilyID	: 8;
	U32 rField1		: 4;
};

struct FeatureSet // contains nothing
{
	union ECX
	{
		struct
		{
			U32 sse3		: 1;
			U32 pclmul		: 1;
			U32 dtse64		: 1;
			U32 monitor		: 1;
			U32 ds_cpl		: 1;
			U32 vmx			: 1;
			U32 smx			: 1;
			U32 eist		: 1;
			U32 tm2			: 1;
			U32 ssse3		: 1;
			U32 cnxt_id		: 1;
			U32 sdbg		: 1;
			U32 fma			: 1;
			U32 cmpxchg16b	: 1;
			U32 xTPRUpdateControl	: 1;
			U32 pdcm		: 1;
			U32 rfield0		: 1;
			U32 pcid		: 1;
			U32 directCacheAccess	: 1;
			U32 sse4_1		: 1;
			U32 sse4_2		: 1;
			U32 x2APIC		: 1;
			U32 movbe		: 1;
			U32 popCNT		: 1;
			U32 tscDeadline	: 1;
			U32 aes			: 1;
			U32 xsave		: 1;
			U32 osxsave		: 1;
			U32 avx			: 1;
			U32 f16c		: 1;
			U32 rdrand		: 1;
		};
		U32 fset;
	};

	union EDX
	{
		struct
		{
			U32 x87FPU			: 1;
			U32 vme				: 1;
			U32 debuggingExt	: 1;
			U32 pageSizeExt		: 1;
			U32 timeStampCounter: 1;
			U32 msrSupport		: 1;
			U32 physAddrEnt		: 1;
			U32 machineCheckExp	: 1;
			U32 cx8				: 1;
			U32 apic			: 1;
			U32 rField0			: 1;
			U32 sysEnterAndExit	: 1;
			U32 memTypeRangeRegs: 1;
			U32 pgeAndPTEGlobalBits	: 1;
			U32 machineCheckArch	: 1;
			U32 cmov			: 1;
			U32 pageAttrTable	: 1;
			U32 pageSizeExt36	: 1;
			U32 serialNumber	: 1;
			U32 cflushInt		: 1;
			U32 rField1			: 1;
			U32 debugStore		: 1;
			U32 acpiThermAndClkCtl	: 1;
			U32 mmxTech			: 1;
			U32 fxsr			: 1;
			U32 sse				: 1;
			U32 sse2			: 1;
			U32 selfSnoop		: 1;
			U32 htt				: 1;
			U32 thermalMonitor	: 1;
			U32 pendBrkEN		: 1;
		};
		U32 fset;
	};
};

}//ns intel
}// cpuid
}// hal

import_asm unsigned long TestCPUID(void);
import_asm void __cpuid(U32 EAX, U32 ECX, U32 *destinationPointer);

#endif/* HAL/CPUID.h */
