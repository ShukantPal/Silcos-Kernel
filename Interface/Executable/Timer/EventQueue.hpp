///
/// @file EventQueue.hpp
/// -------------------------------------------------------------------
/// This program is free software: you can redistribute it and/or modify
/// it under the terms of the GNU General Public License as published by
/// the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program.  If not, see <http://www.gnu.org/licenses/>
///
/// Copyright (C) 2017 - Shukant Pal
///

#ifndef EVENTQUEUE_HPP_
#define EVENTQUEUE_HPP_

#include "Event.hpp"
#include "EventTrigger.hpp"
#include "EventNode.hpp"
#include "NodeSorter.hpp"
#include <Utils/ArrayList.hpp>
#include <Utils/RBTree.hpp>

namespace Executable
{
namespace Timer
{

class EventQueue
{
public:
	EventQueue();
	void strain(EventTrigger *timer);
	void release(EventTrigger *timer);
	EventNode *invokePull();
private:
	NodeSorter nTree;
};

}// namespace Timer
}// namespace Executable

#endif/* Executable/Timer/EventQueue.hpp */
