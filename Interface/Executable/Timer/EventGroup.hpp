/**
 * @file EventNode.hpp
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
#ifndef EXECMGR_TIMER_EVENTGROUP_HPP_
#define EXECMGR_TIMER_EVENTGROUP_HPP_

#include <Executable/Timer/Event.hpp>
#include <Memory/KObjectManager.h>

namespace Executable
{
namespace Timer
{

/**
 * Slab allocator for <tt>Executable::Timer::EventNode</tt>
 */
extern struct ObjectInfo *t_EventNode;

/**
 * Possible colors assigned to <tt>EventGroup</tt> objects while
 * stored sorted in a <tt>Timeline</tt> container.
 */
enum NodeColor
{
	kRed, //!< kRed
	kBlack//!< kBlack
};

/**
 * Groups overlapping events, so that they can be executed together
 * in a timer. <tt>Event</tt> objects are stored together in an
 * array, allowing events to be added, removed, and paused.
 *
 * TODO:
 * Form a model for merging two <tt>EventGroup</tt> objects.
 */
class EventGroup final
{
public:
	EventGroup(Timestamp trigger, Delay shiftAllowed,
			EventCallback handler, void *eventObject);
	Event* add(Timestamp trigger, Delay shiftAllowed,
			EventCallback handler, void *eventObject);
	void del(Event *trig, Timestamp (&impendingRange)[2]);
	bool isHoldable(Timestamp rangeStart, Timestamp rangeEnd);

	Timestamp *getCurrentRange() {
		return (overlapRange);
	}

	Event *getTriggerByIndex(unsigned int index) {
		return (etrigArray + index);
	}
private:
public:
	Event *etrigArray;
	unsigned long etrigCount;
	unsigned long holeCount;
	unsigned long bufferSize;
	Timestamp overlapRange[2];

	EventGroup *parent;
	EventGroup *leftChild, *rightChild;
	NodeColor color;

	bool isRed() {
		return (color == kRed);
	}

	bool isBlack() {
		return (color == kBlack);
	}

	bool isLeftChild() {
		return (parent->leftChild == this);
	}

	bool isRightChild() {
		return (parent->rightChild == this);
	}

	void setLeftChild(EventGroup *newLeftChild, EventGroup *urNil) {
		leftChild = newLeftChild;
		if(newLeftChild != urNil)
			newLeftChild->parent = this;
	}

	void setRightChild(EventGroup *newRightChild, EventGroup *urNil) {
		rightChild = newRightChild;
		if(newRightChild != urNil)
			newRightChild->parent = this;
	}

	EventGroup *getSibling() {
		if(isLeftChild())
			return (parent->rightChild);
		else
			return (parent->leftChild);
	}

	void cleanCalculateRange(Timestamp (&newRange)[2]);
	int rematchRange(Timestamp newRange[2]);
	void ensureBuffer(unsigned long requiredCapacity);
	Event *findFreeSlot();

	unsigned int getIndexOf(Event *telem)
	{
		return (telem - etrigArray);
	}

	friend class Timeline;
	friend class NodeSorter;
};

}

}

#endif/* Executable/Timer/EventGroup.hpp */
