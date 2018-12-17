/**
 * @file NodeSorter.cpp
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
 * Copyright (C) 2017, 2018 - Shukant Pal
 */
#include <Executable/Timer/NodeSorter.hpp>
#include <Utils/RBTree.hpp>

using namespace Executable::Timer;

/**
 * Initializes the empty-sorted-tree and allocates a "nil" for this
 * newly constructed tree.
 */
NodeSorter::NodeSorter()
{
	nil = new(t_EventNode) EventGroup(0, 0, 0, 0);

	nil->color = kBlack;
	nil->parent = nil;
	nil->leftChild = nil;
	nil->rightChild = nil;

	treeRoot = nil;
	mMostRecent = nil;
	mMostLate = nil;
	mNodeCount = 0;
}

/**
 * Removes <tt>oldNode</tt> from this tree, <b>without deleting</b>
 * its memory. If it was the most recent or most late node, the next
 * most recent or late node is again calculated.
 *
 * @param oldNode - the node to remove
 */
void NodeSorter::del(EventGroup *oldNode)
{
	if (oldNode == mostRecent()) {
		mMostRecent = findNextMostRecent();
	} else if (oldNode == mostLate()) {
		mMostLate = findNextMostLate();
	}

	if (isNil(oldNode->leftChild) &&
			isNil(oldNode->rightChild)) {
		if(oldNode == treeRoot) {
			treeRoot = nil;
		} else {
			if(oldNode->isRightChild())
				oldNode->parent->rightChild = nil;
			else
				oldNode->parent->leftChild = nil;
		}

		--(mNodeCount);
		return;
	} else if(!isNil(oldNode->leftChild) &&
			!isNil(oldNode->rightChild)) {
		EventGroup *iop = inorderPredecessor(oldNode);
		del(iop);// This has only one child, okay!

		iop->leftChild = oldNode->leftChild;
		iop->rightChild = oldNode->rightChild;
		replaceRoot(oldNode, iop);
		iop->color = oldNode->color;

		--(mNodeCount);
		return;
	}

	/* Here we can assume that oldNode has only one child, and
	   hence, simplify the deletion process. */

	EventGroup *nOnlyChild = (oldNode->leftChild != nil) ?
			oldNode->leftChild : oldNode->rightChild;

	replaceRoot(oldNode, nOnlyChild);

	if(oldNode->color == kBlack) {
		if(nOnlyChild->color == kRed) {
			nOnlyChild->color = kBlack;
		} else {
			fixDeletor(nOnlyChild);
		}
	}

	--(mNodeCount);
}

/**
 * Puts <tt>newElem</tt> in this tree, using <tt>overlapRange[0]</tt>
 * to sort it. It is also checked for being the most recent or most
 * late node.
 *
 * @param newElem - the event-node to insert
 */
void NodeSorter::put(EventGroup *newElem)
{
	if(treeRoot == nil) {
		treeRoot = newElem;
		newElem->color = kBlack;
		newElem->parent = nil;
		newElem->rightChild = nil;
		newElem->leftChild = nil;
		mNodeCount = 1;

		mMostRecent = newElem;
		mMostLate = newElem;
		return;
	}

	getInitialLeaf(newElem, treeRoot);
	repairTree(newElem);

	if(!checkMostRecent(newElem))
		checkMostLate(newElem);

	++(mNodeCount);
}

/**
 * Sets the given "new" node as a leaf at the appropriate position
 * in the tree, without re-balancing the tree/doing any rotations.
 *
 * @param newLeaf - The "new" node added to this sorted tree
 * @param treeRoot - The tree root of this sorted tree
 * @return - The parent of the "new" node after being inserted as
 * 			a leaf.
 */
EventGroup *NodeSorter::getInitialLeaf(EventGroup *newLeaf, EventGroup *treeRoot)
{
	Timestamp rangeStart = newLeaf->overlapRange[0];
	EventGroup *iter = treeRoot, *lparent = nil;

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
		lparent->setLeftChild(newLeaf, nil);
	} else {
		lparent->setRightChild(newLeaf, nil);
	}

	newLeaf->color = kRed;
	newLeaf->leftChild = nil;
	newLeaf->rightChild = nil;

	return (lparent);
}

/**
 * Checks whether a "newly" added leaf becomes the most-recent
 * node.
 *
 * @param newLeaf - The "newly" added leaf to this sorted-tree.
 * @return - Whether this method has set the most-recent node to
 * 			the newLeaf.
 */
bool NodeSorter::checkMostRecent(EventGroup *newLeaf)
{
	if(newLeaf->overlapRange[0] <
			mostRecent()->overlapRange[0]) {
		mMostRecent = newLeaf;
		return (true);
	} else {
		return (false);
	}
}

/**
 * Check whether a "newly" added leaf becomes the most-late
 * node.
 *
 * @param newLeaf - The "newly" added leaf to this sorted tree.
 * @return - Whether this method has set the most-late node to
 * 			the newLeaf.
 */
bool NodeSorter::checkMostLate(EventGroup *newLeaf)
{
	if(newLeaf->overlapRange[0] >
			mostLate()->overlapRange[0]) {
		mMostLate = newLeaf;
		return (true);
	} else {
		return (false);
	}
}

/**
 * Finds the node in this sorted-subtree given, so that the given
 * range can be used to constrain that node, e.g. by adding a
 * trigger with this range. It searches conditionally in both
 * child paths of this subtree using recursion.
 *
 * @param subtree - node from which search should begin (usually root)
 * @param rangeStart - minimum interval time for event
 * @param rangeEnd - maximum interval time for event
 * @return - an node that overlaps suitably with the given time range
 */
EventGroup *NodeSorter::findFor(EventGroup *subtree, Timestamp rangeStart,
		Timestamp rangeEnd)
{
	if(isNil(subtree))
		return (strong_null);

	if(subtree->isHoldable(rangeStart, rangeEnd))
		return (subtree);

	if(!isNil(subtree->leftChild) &&
			rangeStart < subtree->leftChild->overlapRange[1]) {

		EventGroup *lfn = findFor(subtree->leftChild, rangeStart, rangeEnd);
		if(lfn != null)
			return (lfn);
	}

	if(!isNil(subtree->rightChild) &&
			rangeEnd > subtree->rightChild->overlapRange[0]) {

		EventGroup *rfn = findFor(subtree->rightChild, rangeStart, rangeEnd);
		if(rfn != null)
			return (rfn);
	}

	return (strong_null);
}

/**
 * Rotates the sorted sub-tree towards the left, doing standard
 * rotation operations.
 *
 * @param localRoot - The subtree root being rotated
 * @return - The "new" root of the rotated subtree
 */
EventGroup *NodeSorter::leanLeft(EventGroup *localRoot)
{
	EventGroup *newRoot = localRoot->rightChild;
	replaceRoot(localRoot, newRoot);

	localRoot->setRightChild(newRoot->leftChild, nil);
	newRoot->setLeftChild(localRoot, nil);

	return (newRoot);
}

/**
 * Rotates the sorted sub-tree towards the right, doing standard
 * rotation operations.
 *
 * @param localRoot - The subtree root being rotated
 * @return - The "new" root of the rotated subtree
 */
EventGroup *NodeSorter::leanRight(EventGroup *localRoot)
{
	EventGroup *newRoot = localRoot->leftChild;
	replaceRoot(localRoot, newRoot);

	localRoot->setLeftChild(newRoot->rightChild, nil);
	newRoot->setRightChild(localRoot, nil);

	return (newRoot);
}

/**
 * After any node is newly inserted, the sorted tree must be re-balanced
 * using standard red-black tree rotations and re-coloring. This method
 * does so, in one single step.
 *
 * @param newLeaf - The "newly" inserted node/leaf in this sorted-tree.
 * @version 1.0
 */
void NodeSorter::repairTree(EventGroup *newLeaf)
{
	EventGroup *lParent = newLeaf->parent, *lUncle, *gParent;

	if(isNil(lParent)) {
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
			lParent = newLeaf;
			newLeaf = newLeaf->leftChild;
		} else if(newLeaf == gParent->rightChild->leftChild) {
			leanRight(lParent);
			lParent = newLeaf;
			newLeaf = newLeaf->rightChild;
		}

		if(lParent->isLeftChild()) {
			leanRight(gParent);
		} else {
			leanLeft(gParent);
		}

		lParent->color = kBlack;
		gParent->color = kRed;
	}
}

/**
 * Continues the deletion process, on being called in the del()
 * method. It uses recursion, which caused it to be separated into
 * another method.
 *
 * @param nOnlyChild - The child of the node being removed from this
 * 					sorted-tree.
 */
void NodeSorter::fixDeletor(EventGroup *nOnlyChild)
{
	if(isNil(nOnlyChild->parent))
		return;

	EventGroup *ncSibling = nOnlyChild->getSibling();

	if(ncSibling->color == kRed) {
		nOnlyChild->parent->color = kRed;
		ncSibling->color = kBlack;

		if(nOnlyChild->isLeftChild())
			leanLeft(nOnlyChild->parent);
		else
			leanRight(nOnlyChild->parent);
	}

	if(ncSibling->color == kBlack &&
			ncSibling->leftChild->color == kBlack &&
			ncSibling->rightChild->color == kBlack) {
		if(nOnlyChild->parent->color == kBlack) {
			ncSibling->color = kRed;
			fixDeletor(nOnlyChild->parent);
		} else {
			ncSibling->color = kRed;
			nOnlyChild->parent->color = kBlack;
		}
	}

	if(ncSibling->color == kBlack) {
		if(nOnlyChild->isLeftChild() &&
				ncSibling->rightChild->color == kBlack &&
				ncSibling->leftChild->color == kRed) {
			ncSibling->color = kRed;
			ncSibling->leftChild->color = kBlack;
			leanRight(ncSibling);
		} else if(nOnlyChild->isRightChild() &&
				ncSibling->leftChild->color == kBlack &&
				ncSibling->rightChild->color == kRed) {
			ncSibling->color = kRed;
			ncSibling->rightChild->color = kBlack;
			leanLeft(ncSibling);
		}
	}

	ncSibling->color = nOnlyChild->parent->color;
	nOnlyChild->parent->color = kBlack;

	if(nOnlyChild->isLeftChild()) {
		ncSibling->rightChild->color = kBlack;
		leanLeft(nOnlyChild->parent);
	} else {
		ncSibling->leftChild->color = kBlack;
		leanRight(nOnlyChild->parent);
	}
}
