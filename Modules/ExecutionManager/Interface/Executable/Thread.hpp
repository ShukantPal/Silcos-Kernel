/**
 * File: EThread.hpp
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

#ifndef MODULES_EXECUTIONMANAGER_INTERFACE_ETHREAD_HPP_
#define MODULES_EXECUTIONMANAGER_INTERFACE_ETHREAD_HPP_

#include "Task.hpp"

namespace Executable
{

extern "C" {
	typedef unsigned long (*ThreadStartup)();
}

class Thread : public Task
{
public:
	Thread *getThread(ThreadStartup *mainFunc, unsigned long priority);
};

}

#endif/* Executable/Thread.hpp */
