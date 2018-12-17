/**
 * File: FACS.h
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
#ifndef ACPI_FACS_H__
#define ACPI_FACS_H__

#include <TYPE.h>

struct FACS
{
	char signature[4];
	U32 length;
	U32 hardwareSignature;
	U32 wakingVector;

	struct Lock
	{
		U32 Pending	: 1;
		U32 Owned	: 1;
		U32 RESERVED	: 30;
	} globalLock;

#define S4BIOS_F			0
#define _64BIT_WAKE_SUPPORTED_F		1
	U32 fmwrControlFlags;
	U64 xWakingVector;
	U8 version;
	U8 reservedField0[3];
#define _64BIT_WAKE_F			0
	U32 ospmControlFlags;
};

#endif/* ACPI/FACS.h */
