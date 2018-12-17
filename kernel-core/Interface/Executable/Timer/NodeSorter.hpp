/**
 * @file NodeSorter.hpp
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

#include <Executable/Timer/EventGroup.hpp>

namespace Executable
{
namespace Timer
{

/**
 * Sorts <tt>EventNode</tt> objects in a red-black tree, caching the
 * most recent & most late nodes. Nodes are sorted by their starting
 * overlapRange (<tt>node->overlapRange[0]</tt>), and the user must
 * prevent putting any overlapping nodes.
 */
class NodeSorter
{
public:
	inline unsigned nodeCount() {
		return (mNodeCount);
	}

	inline EventGroup *mostRecent() {
		return (mMostRecent);
	}

	inline EventGroup *mostLate() {
		return (mMostLate);
	}

	inline EventGroup *findFor(Timestamp rangeStart,
			Timestamp rangeEnd){
		return findFor(treeRoot, rangeStart, rangeEnd);
	}

	inline EventGroup *nilo() {
		return (nil);
	}

	inline EventGroup *rooto() {
		return (this->treeRoot);
	}

	NodeSorter();
	void del(EventGroup *oldNode);
	void put(EventGroup *newNode);
private:
	EventGroup *treeRoot, *nil;
	EventGroup *mMostRecent, *mMostLate;
	unsigned long mNodeCount;

	EventGroup *getInitialLeaf(EventGroup *newLeaf,
			EventGroup *treeRoot);
	bool checkMostRecent(EventGroup *newLeaf);
	bool checkMostLate(EventGroup *newLeaf);
	EventGroup *findFor(EventGroup *subtree,
			Timestamp rangeStart, Timestamp rangeEnd);
	EventGroup *findMostRecent();
	EventGroup *findMostLate();
	EventGroup *leanLeft(EventGroup *localRoot);
	EventGroup *leanRight(EventGroup *localRoot);
	void repairTree(EventGroup *newLeaf);
	void fixDeletor(EventGroup *nOnlyChild);

	EventGroup *findNextMostRecent() {
		if(isNil(mostRecent()->rightChild)) {
			if(mostRecent() != treeRoot) {
				return (mostRecent()->parent);
			} else {
				return (nil);
			}
		} else {
			 return (inorderSuccessor(mostRecent()));
		}
	}

	EventGroup *findNextMostLate() {
		if(isNil(mostLate()->leftChild)) {
			if(mostLate() != treeRoot) {
				return (mostLate()->parent);
			} else {
				return (nil);
			}
		} else {
			return (inorderPredecessor(mostLate()));
		}
	}

	bool isNil(EventGroup *en) {
		return (en == nil);
	}

	/* Assumes en->leftChild != nil */
	EventGroup *inorderPredecessor(EventGroup *en) {
		en = en->leftChild;
		while(en->rightChild != nil)
			en = en->rightChild;
		return (en);
	}

	/* Assumes en->rightChild != nil */
	EventGroup *inorderSuccessor(EventGroup *en) {
		en = en->rightChild;
		while(en->leftChild != nil)
			en = en->leftChild;
		return (en);
	}

	void replaceRoot(EventGroup *old, EventGroup *with) {
		with->parent = old->parent;

		EventGroup *nParent = old->parent;
		if(nParent != nil) {
			if(old->isLeftChild())
				nParent->leftChild = with;
			else
				nParent->rightChild = with;
		} else {
			treeRoot = with;
		}
	}

	friend class Timeline;
};

} // namespace Timer
} // namespace Executable

#endif/* Executable/Timer/NodeSorter.hpp */
