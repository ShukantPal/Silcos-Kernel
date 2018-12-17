/**
 * @file ACPI.h
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

#ifndef INTERFACE_ACPI_ACPI_H_
#define INTERFACE_ACPI_ACPI_H_

#include <TYPE.h>

namespace ACPI
{

enum PMProfile
{
	Unspecified		= 0,
	Desktop			= 1,
	Mobil			= 2,
	Workstation		= 3,
	EnterpriseServer	= 4,
	SOHOServer		= 5,
	AppliancePC		= 6,
	PerformanceServer	= 7,
	Tablet			= 8
};

enum AddressSpace
{
	SystemMemory		= 0,
	SystemIO		= 1,
	PCIConfig		= 2,
	EmbeddedController	= 3,
	SMBus			= 4,
	PCC			= 5,
	FunctionalFixedHw	= 0x7F
};

struct PCIConfigAddress
{
	U16 offset;
	U16 functionNumber;
	U16 deviceNumer;
	U16 reservedField0;
};

/**
 * @struct GAS
 *
 * Summary:
 * GAS (generic address structure) provides the platform with a means to
 * describe register locations. It is used in ACPI tables to express
 * register addresses.
 *
 * @version 2.0
 * @since Silcos 3.02
 * @author Shukant Pal
 */
struct GAS
{
	U8 addressSpaceID;// address space where data/register exists
	U8 registerBitWidth;// size in bits of the given register (0 for structs)
	U8 registerBitOffset;// bit offset of the given register at address ("")
	U8 accessSize;// specifies the access size to the register/data
	U64 addressValue;// 64-bit address of data/register in given addr-space
} __attribute__((packed));

}

#endif /* INTERFACE_ACPI_ACPI_H_ */
