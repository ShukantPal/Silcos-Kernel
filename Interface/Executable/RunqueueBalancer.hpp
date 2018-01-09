/**
 * File: RunqueueBalancer.hpp
 *
 * Summary:
 * Frames the runqueue-balancing mechanism b/w diff. sibling domains, allowing
 * the distribution of tasks in a uniform manner.
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
#ifndef INTERFACE_EXECUTABLE_RUNQUEUEBALANCER_HPP_
#define INTERFACE_EXECUTABLE_RUNQUEUEBALANCER_HPP_

#include <Executable/ScheduleRoller.h>
#include <HAL/ProcessorTopology.hpp>

struct ObjectInfo;
extern ObjectInfo *tRunqueueBalancer_Accept;
extern ObjectInfo *tRunqueueBalancer_Renounce;
struct Processor;

namespace Executable
{

class RunqueueBalancer final
{
public:
	static void init();
	static void balanceWork(ScheduleClass cls) kxhide;
	static void balanceWork(ScheduleClass cls, HAL::Domain *dom) kxhide;

	struct Accept : public HAL::IPIRequest
	{
		const ScheduleClass taskType;
		CircularList taskList;
		HAL::Domain &donor, &taker;// domain giving the tasks
		unsigned long load;

		Accept(ScheduleClass cls, HAL::Domain &tsource, HAL::Domain &client)
		: HAL::IPIRequest(HAL::AcceptTasks, 0, tRunqueueBalancer_Accept),
			taskType(cls), donor(tsource), taker(client)
		{
			load = 0;
		}
	};

	struct Renounce : public HAL::IPIRequest
	{
		const ScheduleClass taskType;
		HAL::Domain &donor, &taker;
		Processor &src, &dst;

		Renounce(ScheduleClass cls, HAL::Domain &from, HAL::Domain &to,
				Processor &src_, Processor &dst_)
		: HAL::IPIRequest(HAL::RenounceTasks, 0, tRunqueueBalancer_Renounce),
		  	  taskType(cls), donor(from), taker(to), src(src_), dst(dst_)
		{

		}
	};

private:
	RunqueueBalancer() kxhide;
};

}// namespace Executable

#endif/* Executable/RunqueueBalancer.hpp */
