/**
 * @file NodeSorter.cpp
 *
 * Implements a red-black tree that caches the first and last in-order
 * elements. This is particularly useful for the EventQueue. This separate
 * implementation was required due to the 64-bit key of the Timestamp.
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
#include <Executable/Timer/NodeSorter.hpp>
#include <Utils/RBTree.hpp>

using namespace Executable::Timer;

NodeSorter::NodeSorter()
{
	nil = new(t_EventNode) EventNode(0, 0, 0, 0);

	nil->color = kBlack;
	nil->parent = nil;
	nil->leftChild = nil;
	nil->rightChild = nil;

	treeRoot = null;
	mostRecent = null;
	mostLate = null;
	nodeCount = 0;
}

EventNode *NodeSorter::findFor(EventTrigger *constraint)
{
	EventNode *iter = treeRoot, *closest;
	Timestamp rangeStart = constraint->triggerRange[0];

	while(iter != nil) {
		if(iter->overlapRange[1] < rangeStart) {
			iter = iter->rightChild;
		} else if(iter->overlapRange[0] > rangeStart) {
			iter = iter->leftChild;
		} else {
			return (iter);
		}
	}

	return (strong_null);
}

void NodeSorter::put(EventNode *newElem)
{
	if(treeRoot == null) {
		treeRoot = newElem;
		newElem->parent = nil;
		newElem->rightChild = nil;
		newElem->leftChild = nil;
		nodeCount = 1;
		return;
	}

	getInitialLeaf(newElem, treeRoot);
	repairTree(newElem);
}

EventNode *NodeSorter::getInitialLeaf(EventNode *newLeaf, EventNode *treeRoot)
{
	Timestamp rangeStart = newLeaf->overlapRange[0];
	EventNode *iter = treeRoot, *lparent = nil;

	do {
		lparent = iter;

		if(rangeStart < iter->overlapRange[0]) {
			iter = iter->leftChild;
		} else if(rangeStart > iter->overlapRange[0]) {
			iter = iter->rightChild;
		} else {
			return (null);
		}
	} while(iter != nil);

	if(rangeStart < lparent->overlapRange[0]) {
		lparent->setLeftChild(newLeaf);
	} else {
		lparent->setRightChild(newLeaf);
	}

	newLeaf->color = kRed;
	newLeaf->leftChild = nil;
	newLeaf->rightChild = nil;

	return (lparent);
}

EventNode *NodeSorter::leanLeft(EventNode *localRoot)
{
	EventNode *newRoot = localRoot->rightChild;
	newRoot->parent = localRoot->parent;

	localRoot->setRightChild(newRoot->leftChild);
	newRoot->setLeftChild(localRoot);

	return (newRoot);
}

EventNode *NodeSorter::leanRight(EventNode *localRoot)
{
	EventNode *newRoot = localRoot->leftChild;
	newRoot->parent = localRoot->parent;

	localRoot->setLeftChild(newRoot->rightChild);
	newRoot->setRightChild(localRoot);

	return (newRoot);
}

void NodeSorter::repairTree(EventNode *newLeaf)
{
	EventNode *lParent = newLeaf->parent, *lUncle, *gParent;

	if(lParent == nil) {
		treeRoot = newLeaf;
		newLeaf->color = kBlack;
	} else if(lParent->color == kBlack) {
		return;
	} else if((lUncle = lParent->getSibling())->color == kRed) {
		gParent = lParent->parent;

		lParent->color = kBlack;
		lUncle->color = kBlack;
		gParent->color = kRed;
		repairTree(gParent);
	} else {
		gParent = lParent->parent;

		if(newLeaf == gParent->leftChild->rightChild) {
			leanLeft(lParent);
			newLeaf = newLeaf->leftChild;
		} else if(newLeaf == gParent->rightChild->leftChild) {
			leanRight(lParent);
			newLeaf = newLeaf->rightChild;
		}

		if(newLeaf->isLeftChild())
			leanRight(gParent);
		else
			leanLeft(gParent);

		lParent->color = kBlack;
		gParent->color = kRed;
	}
}
