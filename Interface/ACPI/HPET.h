/**
 * File: HPET.h
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
#ifndef HAL_ACPI_HPET_H__
#define HAL_ACPI_HPET_H__

#include "ACPI.h"
#include "SDTHeader.h"
#include <TYPE.h>

namespace ACPI
{

/* @struct HPET
 *
 * Provides information to the OS about the HPET in the system to allow
 * it to boot-strap. Only the 1st timer-block information is given in the form
 * of ACPI tables, while others are reported in the ACPI namespace.
 *
 * @version 2.0
 * @since Silcos 3.02
 * @author Shukant Pal
 */
struct HPET : public SDTHeader
{
	struct EventTimerBlockID
	{
		U32 hwRevisionID : 8;
		U32 comparatorCount : 5;
		U32 counterSize : 1;
		U32 reservedField0 : 1;
		U32 legacyRouting : 1;
		U32 vendorID : 16;
	} blockID;

	GAS baseAddress;
	U8 timerNumber;
	U16 mcMinTicks;

	enum PageProtectionLevel
	{
		NoGuarantee = 0,// no guarantee for page protection
		PP4K = 1,	// 4K-block protection, access does not compromise security
		PP64K = 2	// 64K-block protection, access does not compromise security
	};

	struct AttributeSet
	{
		U8 ppLevel : 4;
		U8 oemAttr : 4;
	} attributes;
} __attribute__((packed));

}

decl_c void testhpet();

#endif/* ACPI/HPET.h */
