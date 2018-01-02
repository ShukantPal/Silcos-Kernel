/*=++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * File: MSR.h
 *
 * Summary: This file contains all architectural MSRs and the more 'popular' MSRs are given
 * a seperate type according to their structure.
 *
 * Copyright (C) 2017 - Shukant
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=*/
#ifndef IA32_MSR_H
#define IA32_MSR_H

#include <TYPE.h>

#define IA32_P5_MC_ADDR				0x0
#define IA32_P5_MC_TYPE				0x1
#define IA32_MONITOR_FILTER_SIZE 		0x6
#define IA32_TIME_STAMP_COUNTER		0x10
#define IA32_PLATFORM_ID				0x17
#define IA32_APIC_BASE				0x1B
#define IA32_FEATURE_CONTROL			0x3A
#define IA32_TSC_ADJUST				0x3B
#define IA32_BIOS_UPDT_TRIG			0x79
#define IA32_BIOS_SIGN_ID			0x8B
#define IA32_SGXLEPUBKEYHASHD			0x8C
#define IA32_SGXLEPUBKEYHASH1			0x8D
#define IA32_SGXLEPUBKEYHASH2			0x8E
#define IA32_SGXLEPUBKEYHASH3			0x8F
#define IA32_SMM_MONTIOR_CTL			0x9B
#define IA32_SMBASE					0x9E
#define IA32_PMC0					0xC1
#define IA32_PMC1					0xC2
#define IA32_PMC2					0xC3
#define IA32_PMC3					0xC4
#define IA32_PMC4					0xC5
#define IA32_PMC5					0xC6
#define IA32_PMC6					0xC7
#define IA32_PMC7					0xC8
#define IA32_MPERF					0xE7
#define IA32_APERF					0xEB
#define IA32_MTRRCAP				0xFE
#define IA32_SYSENTER_CS				0x174
#define IA32_SYSENTER_ESP			0x175
#define IA32_SYSENTER_EIP			0x176
#define IA32_SYSENTER_MCG_CAP			0x179
#define IA32_MCG_STATUS				0x17A
#define IA32_MCG_CTL				0x17B
#define IA32_PERFEVTSEL0				0x186
#define IA32_PERFEVTSEL1				0x187
#define IA32_PERFEVTSEL2				0x188
#define IA32_PERFEVTSEL3				0x189
#define IA32_PERF_STATUS				0x198
#define IA32_PERF_CTL				0x199
#define IA32_CLOCK_MODULATION			0x19A
#define IA32_THERM_INTERRUPT			0x19B
#define IA32_THERM_STATUS			0x19C
#define IA32_MISC_ENABLE				0x1A0
#define IA32_ENERGY_PERF_BIAS			0x1B0
#define IA32_PACKAGE_THERM_INTERRUPT	0x1B2
#define IA32_DEBUGCTL				0x1D9
#define IA32_SMRR_PHYSBASE			0x1F2
#define IA32_SMRR_PHYSMASK			0x1F3
#define IA32_PLATFORM_DCA_CAP			0x1F8
#define IA32_CPU_DCA_CAP				0x1F9
#define IA32_DCA_0_CAP				0x1FA
#define IA32_MTRR_PHYSBASE0			0x200
#define IA32_MTRR_PHYSMASK0			0x201
#define IA32_MTRR_PHYSBASE1			0x202
#define IA32_MTRR_PHYSMASK1			0x203
#define IA32_MTRR_PHYSBASE2			0x204
#define IA32_MTRR_PHYSMASK2			0x205
#define IA32_MTRR_PHYSBASE3			0x206
#define IA32_MTRR_PHYSMASK3			0x207
#define IA32_MTRR_PHYSBASE4			0x208
#define IA32_MTRR_PHYSMASK4			0x209
#define IA32_MTRR_PHYSBASE5			0x20A
#define IA32_MTRR_PHYSMASK5			0x20B
#define IA32_MTRR_PHYSBASE6			0x20C
#define IA32_MTRR_PHYSMASK6			0x20D
#define IA32_MTRR_PHYSBASE7			0x20E
#define IA32_MTRR_PHYSMASK7			0x20F
#define IA32_MTRR_PHYSBASE8			0x210
#define IA32_MTRR_PHYSMASK8			0x211
#define IA32_MTRR_PHYSBASE9			0x212
#define IA32_MTRR_PHYSMASK9			0x213
#define IA32_MTRR_FIX64K_00000		0x250
#define IA32_MTRR_FIX16K_80000		0x258
#define IA32_MTRR_FIX16K_A0000		0x259
#define IA32_MTRR_FIX4K_C0000			0x268
#define IA32_MTRR_FIX4K_C8000			0x269
#define IA32_MTRR_FIX4K_D0000			0x26A
#define IA32_MTRR_FIX4K_D8000			0x26B
#define IA32_MTRR_FIX4K_E0000			0x26C
#define IA32_MTRR_FIX4K_E8000			0x26D
#define IA32_MTRR_FIX4K_F0000			0x26E
#define IA32_MTRR_FIX4K_F8000			0x26F
#define IA32_PAT					0x277
#define IA32_MC0_CTL2				0x280
#define IA32_MC1_CTL2				0x281
#define IA32_MC2_CTL2				0x282
// TODO: Complete IA32_MC%d_CTL2 till 31
#define IA32_MTRR_DEF_TYPE			0x2FF
#define IA32_FIXED_CTR0						0x309
#define IA32_FIXED_CTR1						0x30A
#define IA32_FIXED_CTR2						0x30B
#define IA32_PERF_CAPABILITIES			0x345
#define IA32_FIXED_CTR_CTRL				0x38D
#define IA32_PERF_GLOBAL_STATUS	0x38E
#define IA32_PERF_GLOBAL_CTRL		0x38F
#define IA32_PERF_GLOBAL_OVF_CTRL			0x390
#define IA32_PERF_GLOBAL_STATUS_RESET 0x390
#define IA32_PERF_GLOBAL_STATUS_SET		 0x391
// TODO: Complete IA32_MC%d_CTL, STATUS, ADDR, MISC, CTL till 28
// TODO: Complete VMX msr
// TODO: Complete IA32_A_PMC%d till 7
#define IA32_MCG_EXT_CTL					0x4D0
#define IA32_SGX_SVN_STATUS			0x500
#define IA32_RTIT_OUTPUT_BASE		0x560
#define IA32_RTIT_OUTPUT_MASK_PTRS	0x561
#define IA32_RTIT_CTL							0x570
#define IA32_RTIT_STATUS					0x571
// Complte RTIT
#define IA32_TSC_DEADLINE					0x6E0
#define IA32_PM_ENABLE						0x770
#define IA32_HWP_CAPABILITIES			0x771
#define IA32_HWP_REQUEST_PKG		0x772
#define IA32_HWP_INTERRUPT				0x773
#define IA32_HWP_REQUEST				0x774
#define IA32_HWP_STATUS							0x777
#define IA32_X2APIC_APICID                    0x802
#define IA32_X2APIC_VERSION       0x803
#define IA32_X2APIC_TPR                0x804
#define IA32_X2APIC_PPR                0x805
#define IA32_X2APIC_EOI                 0x80B
#define IA32_X2APIC_LDR                0x80D
#define IA32_X2APIC_SIVR               0x80F
#define IA32_X2APIC_ISR                 0x810
#define IA32_X2APIC_TMR               0x818
#define IA32_X2APIC_IRR                 0x820
#define IA32_X2APIC_ESR                0x828
#define IA32_X2APIC_LVT_TIMER  0x832
#define IA32_X2APIC_LVT_THERMAL 0x833
#define IA32_X2APIC_LVT_PERFORMANCE_MONITORING 0x834
#define IA32_X2APIC_LVT_LINT0   0x835
#define IA32_X2APIC_LVT_LINT1   0x836
#define IA32_X2APIC_LVT_ERROR  0x837
#define IA32_X2APIC_TIMER_ICR   0x838
#define IA32_X2APIC_TIMER_CCR  0x839
#define IA32_X2APIC_TIMER_DCR  0x840
#define IA32_X2APIC_SELF_IPI         0x83F
#define IA32_DEBUG_INTERFACE					0xC80
#define IA32_L3_QOS_CFG							0xC81
#define IA32_AM_CTR									0xCBE
#define IA32_PGR_ASSOC								0xC8F
#define IA32_L3_MASK_0								0xC90
#define IA32_L3_MASK(n)								IA32_L3_MASK + n

namespace IA32
{

typedef
union APICBaseMSR
{
	struct
	{
		U64 R1		: 8;
		U64 BSP		: 1;/* Set if this is the BSP */
		U64 R2		: 1;
		U64 EXTD	: 1;/* x2APIC Extension Flag */
		U64 EN		: 1;/* Global APIC Enable/Disable Flag s*/
		U64 apicBase	: 24;/* Base of xAPIC registers */
		U64 RESERVED	: 28;
	};
	U64 msrValue;
} IA32_APIC_BASE_MSR;

}

static inline
void ReadMSR(U32 msrOffset, U64 *msrValue){
	asm volatile("RDMSR" : "=a"(*((U32*) msrValue)), "=d"(*((U32*) ((UBYTE*) msrValue + sizeof(U32)))) : "c"(msrOffset));
}

static inline
void WriteMSR(U32 msrOffset, U64 msrValue){
	asm volatile("WRMSR" : : "a"((U32) msrValue), "d"(*((U32*) ((UBYTE*) &msrValue + sizeof(U32)))), "c"(msrOffset));
}

#endif/* IA32/MSR.h */
