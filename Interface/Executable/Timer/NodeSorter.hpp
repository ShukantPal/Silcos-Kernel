/**
 * @file NodeSorter.hpp
 *
 * Provides a container for holding EventNode objects in a sorted
 * data structure - particularly, the red-black tree. This container
 * fulfills the requirement of having cached minimum and maximum nodes
 * so that the EventQueue object can easily push and pull time ranges.
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
#ifndef EXECMGR_TIMER_NODESORTER_HPP__
#define EXECMGR_TIMER_NODESORTER_HPP__

#include "EventNode.hpp"

namespace Executable
{
namespace Timer
{

class NodeSorter
{
public:
	inline EventNode *getMostRecent() {
		return (mostRecent);
	}

	inline EventNode *getMostLate() {
		return (mostLate);
	}

	inline EventNode *findFor(Timestamp rangeStart,
			Timestamp rangeEnd){
		return findFor(treeRoot, rangeStart, rangeEnd);
	}

	NodeSorter();
	void del(EventNode *oldNode);
	void put(EventNode *newNode);
private:
	EventNode *treeRoot, *nil;
	EventNode *mostRecent, *mostLate;
	unsigned long nodeCount;

	EventNode *getInitialLeaf(EventNode *newLeaf,
			EventNode *treeRoot);
	bool checkMostRecent(EventNode *newLeaf);
	bool checkMostLate(EventNode *newLeaf);
	EventNode *findFor(EventNode *subtree,
			Timestamp rangeStart, Timestamp rangeEnd);
	EventNode *findMostRecent();
	EventNode *findMostLate();
	EventNode *leanLeft(EventNode *localRoot);
	EventNode *leanRight(EventNode *localRoot);
	void repairTree(EventNode *newLeaf);
	void fixDeletor(EventNode *nOnlyChild);

	EventNode *findNextMostRecent() {
		if(getMostRecent() != treeRoot)
			return (getMostRecent()->parent);
		else
			return (getMostRecent()->rightChild);
	}

	EventNode *findNextMostLate() {
		return (getMostLate()->parent);
	}

	bool isNil(EventNode *en) {
		return (en == nil);
	}

	bool isMostRecent(EventNode *en) {
		return (en == getMostRecent());
	}

	bool isMostLate(EventNode *en) {
		return (en == getMostLate());
	}

	/* Find the inorder predecessor, assuming left-subtree
	   exists. */
	EventNode *inorderPredecessor(EventNode *en) {
		en = en->leftChild;
		while(en->rightChild != nil)
			en = en->rightChild;
		return (en);
	}

	/* Find the inorder successor, assuming right-subtree
	   exists. */
	EventNode *inorderSuccessor(EventNode *en) {
		en = en->rightChild;
		while(en->leftChild != nil)
			en = en->leftChild;
		return (en);
	}

	void setMostRecent(EventNode *newExtreme) {
		mostRecent = newExtreme;
	}

	void setMostLate(EventNode *newExtreme) {
		mostLate = newExtreme;
	}

	void replaceRoot(EventNode *old, EventNode *with) {
		with->parent = old->parent;

		EventNode *nParent = old->parent;
		if(nParent != nil) {
			if(old->isLeftChild())
				nParent->leftChild = with;
			else
				nParent->rightChild = with;
		} else {
			treeRoot = with;
		}
	}

	friend class EventQueue;
};

} // namespace Timer
} // namespace Executable

#endif/* Executable/Timer/NodeSorter.hpp */
