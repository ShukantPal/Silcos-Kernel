/**
 * File: FADT.h
 *
 * Summary:
 * 
 * Functions:
 *
 * Origin:
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

#ifndef INTERFACE_ACPI_FADT_H_
#define INTERFACE_ACPI_FADT_H_

#include "SDTHeader.h"
#include <TYPE.h>

namespace ACPI
{

/**
 * @struct FADT
 *
 * The fixed ACPI description-table defines various fixed hardware
 * ACPI information. It provides base addresses for essential control
 * register blocks.
 *
 * See: ACPI Specifications
 */
struct FADT : public SDTHeader
{
	U32 firmwareControl;// physical address of FACS
	U32 dsdtAddr;// physical memory address for the DSDT
	U8 reservedField0;
	U8 prefPMProfile;// specifies the preferred power-management profile
	U16 sciVect;
	U32 smiCmdAddress;// system port address of the SMI Command Port
	U8 acpiEnable;// value to write to SMI_CMD to disable SMI ownership
	U8 acpiDisable;// value to write to SMI_CMD to re-enable SMI ownership
	U8 S4BIOSRequest;// value to write to SMI_CMD to enter S4BIOS state
	U8 pstateCount;
#define PM1a_EVT_BLK	0
#define PM1b_EVT_BLK	1
#define PM1a_CNT_BLK	2
#define PM1b_CNT_BLK	3
#define PM2_CNT_BLK	4
#define PM_TMR_BLK	5
#define GPE0_BLK	6
#define GPE1_BLK	7
	U32 registerPorts[8];
#define PM1_EVT_LEN	0
#define PM1_CNT_LEN	1
#define PM2_CNT_LEN	2
#define PM_TMR_LEN	3
#define GPE0_BLK_LEN	4
#define GPE1_BLK_LEN	5
	U8 registerSizes[6];
	U8 gpe1Base;
	U8 CST_CNT;
	U16 plevel2Latency;
	U16 plevel3Latency;
	U16 flushSize;
	U16 flushStride;
	U8 dutyOffset;
	U8 dutyWidth;
	U8 dayAlarm;
	U8 monthAlarm;
	U8 century;
#define LEGACY_DEVICES				0
#define SUPPORT_8042				1
#define VGA_NOT_PRESENT				2
#define MSI_NOT_SUPPORTED			3
#define PCIe_ASPM_CONTROLS			4
#define CMOS_RTC_NOT_PRESENT			5
	U16 bootFlags;
	U8 reservedField2;
#define WBINVD					0
#define WBINVD_FLUSH				1
#define PROC_C1					2
#define P_LVL2_UP				3
#define PWR_BUTTON				4
#define SLP_BUTTON				5
#define FIX_RTC					6
#define RTC_S4					7
#define TMR_VAL_EXT				8
#define DCK_CAP					9
#define RESET_REG_SUP				10
#define SEALED_CASE				11
#define HEADLESS				12
#define CPU_SW_SLP				13
#define PC_EXP_WAK				14
#define USE_PLATFORM_CLOCK			15
#define S4_RTC_STS_VALID			16
#define REMOTE_POWER_ON_CAPABLE			17
#define FORCE_APIC_CLUSTER_MODEL		18
#define FORCE_APIC_PHYSICAL_DESTINATION_MODE	19
#define HW_REDUCED_ACPI				20
#define LOW_POWER_S0_IDLE_CAPABLE		21
	U32 fixedFeatureFlags;
	GAS resetRegister;
	U8 resetValue;
	U8 reservedField1[3];
	U64 xFirmwareControl;
	U64 xDSDT;
	GAS xRegisterPorts[8];
	GAS sleepControlReg;
	GAS sleepStatusReg;
};

}

#endif /* INTERFACE_ACPI_FADT_H_ */
