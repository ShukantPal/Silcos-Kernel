/**
 * @file Timeline.hpp
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
#ifndef EXECMGR_TIMER_TIMELINE_HPP_
#define EXECMGR_TIMER_TIMELINE_HPP_

#include <Executable/Timer/Event.hpp>
#include <Executable/Timer/EventGroup.hpp>
#include "NodeSorter.hpp"
#include <Utils/ArrayList.hpp>
#include <Utils/RBTree.hpp>

namespace Executable
{
namespace Timer
{

class Timeline
{
public:
	inline void addAll(EventGroup *group) {
		ndsEngine.put(group);
	}

	inline unsigned nodalCount() {
		return (ndsEngine.nodeCount());
	}

	inline bool isEmpty() {
		return (nodalCount() == 0);
	}

	inline EventGroup *firstGroup() {
		return (ndsEngine.mostRecent());
	}

	inline EventGroup *get() {
		EventGroup *mr = ndsEngine.mostRecent();

		if(ndsEngine.isNil(mr))
			return (strong_null);
		else {
			ndsEngine.del(mr);
			return (mr);
		}
	}

	inline NodeSorter *ns() {
		return (&ndsEngine);
	}

	inline void add(EventGroup *newGroup) {
		ndsEngine.put(newGroup);
	}

	Timeline();
	Event *add(Timestamp trigger,
			Timestamp shiftAllowed, EventCallback handler,
			void *eventObject);
	bool rem(Event *timer);
private:
	NodeSorter ndsEngine;
};

}// namespace Timer
}// namespace Executable

#endif/* Executable/Timer/EventQueue.hpp */
