/**
 * @file Environment.h
 *
 * Allows kernel subsystem to know about the kernel environment
 * parameters. This is in a developmental state, and only the
 * minimal features have been included.
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
#ifndef INTERFACE_ENVIRONMENT_H_
#define INTERFACE_ENVIRONMENT_H_

#include <Memory/Pager.h>

struct Environment
{
	PhysAddr loadAddress;
	PhysAddr loadSize;
	PhysAddr pageframeEntries;
	PhysAddr multibootTable;
	unsigned long bootModules;/* filled later in Main() */
};

extern Environment kernelParams;


#endif/* Environment.h */
