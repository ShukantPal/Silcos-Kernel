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
#ifndef EVENTNODE_HPP_
#define EVENTNODE_HPP_

#include "Event.hpp"
#include "EventTrigger.hpp"
#include <Memory/KObjectManager.h>

namespace Executable
{
namespace Timer
{

extern struct ObjectInfo *t_EventNode;

enum NodeColor
{
	kRed,
	kBlack
};

/**
 * Holds an dynamic array of EventTrigger objects that can be executed
 * at the same time (.i.e. in one interrupt), grouping them together so
 * that interrupt overhead can be reduced.
 *
 * Cancellation of EventTrigger objects can cause complexity in insertion,
 * deletion, and merge operations in the node. A holed-array model is used
 * where cancelled EventTrigger's are kept in place until their memory can
 * be used to hold another EventTrigger. During merging, filling of holes is
 * done as far as possible.
 *
 * Whenever the trigger-object array is modified, the time-range for the
 * execution of the node is re-calculated. To prevent overhead of multiple
 * memory accesses, the recommended limit for trigger-objects in one node is
 * currently set to 10.
 */
class EventNode final
{
public:
	EventNode(Timestamp trigger, Delay shiftAllowed,
			EventCallback handler, void *eventObject);
	EventTrigger* add(Timestamp trigger, Delay shiftAllowed,
			EventCallback handler, void *eventObject);
	void del(EventTrigger *trig, Timestamp &impendingRange[2]);
	bool isHoldable(Timestamp rangeStart, Timestamp rangeEnd);

	static void init() {
		t_EventNode = KiCreateType("EventNode", sizeof(EventNode), NO_ALIGN,
				null, null);
	}

	Timestamp *getCurrentRange() {
		return (overlapRange);
	}

	EventTrigger *getTriggerByIndex(unsigned int index) {
		return (etrigArray + index);
	}
private:
public:
	EventTrigger *etrigArray;
	unsigned long etrigCount;
	unsigned long holeCount;
	unsigned long bufferSize;
	Timestamp overlapRange[2];

	EventNode *parent;
	EventNode *leftChild, *rightChild;
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

	void setLeftChild(EventNode *newLeftChild, EventNode *urNil) {
		leftChild = newLeftChild;
		if(newLeftChild != urNil)
			newLeftChild->parent = this;
	}

	void setRightChild(EventNode *newRightChild, EventNode *urNil) {
		rightChild = newRightChild;
		if(newRightChild != urNil)
			newRightChild->parent = this;
	}

	EventNode *getSibling() {
		if(isLeftChild())
			return (parent->rightChild);
		else
			return (parent->leftChild);
	}

	void cleanCalculateRange(Timestamp &newRange[2]);
	int rematchRange(Timestamp newRange[2]);
	void ensureBuffer(unsigned long requiredCapacity);
	EventTrigger *findFreeSlot();

	unsigned int getIndexOf(EventTrigger *telem)
	{
		return (telem - etrigArray);
	}

	friend class EventQueue;
	friend class NodeSorter;
};

}
}


#endif /* EVENTNODE_HPP_ */
