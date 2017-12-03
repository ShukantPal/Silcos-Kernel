/**
 * File: EProcess.hpp
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

#ifndef MODULES_RESOURCEMANAGER_INTERFACE_PROCESS_EPROCESS_HPP_
#define MODULES_RESOURCEMANAGER_INTERFACE_PROCESS_EPROCESS_HPP_

class EProcess
{
public:
	inline unsigned long getPID(){ return (pid); }
	inline unsigned long getThreadCount(){ return (threadCount); }
	inline unsigned long getActiveThreadCount(){ return (activeThreadCount); }


protected:
	EProcess();
private:
	unsigned long pid;
	unsigned long threadCount;
	unsigned long activeThreadCount;
};

#endif /* MODULES_RESOURCEMANAGER_INTERFACE_PROCESS_EPROCESS_HPP_ */
