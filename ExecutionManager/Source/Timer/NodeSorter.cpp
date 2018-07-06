/**
 * @file NodeSorter.cpp
 *
 * The NodeSorter internally implements red-black tree which caches the
 * left-most and right-most nodes. This optimization vastly improves
 * the efficiency of interval addition and deletion for event-nodes.
 *
 * In addition to the red-black tree properties, this implementation
 * assumes the following:
 *
 * ^ All event nodes in this tree are non-overlapping, and any node
 *   can be shrinked (in range) without modifying the tree.
 *
 * ^ Even if overlapping does occur - it will be less than 500 ticks
 * (unit of hw-timer concerned).
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
 *
 * @author Shukant Pal
 */
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

/**
 * Deletes the node from the sorted-tree in the normal red-black tree fashion,
 * updating the most-recent & most-late nodes.
 *
 * @param oldNode
 */
void NodeSorter::del(EventNode *oldNode)
{
	if(isMostRecent(oldNode))
		setMostRecent(findNextMostRecent());
	else if(isMostLate(oldNode))
		setMostLate(findNextMostLate());

	if(isNil(oldNode->leftChild) &&
			isNil(oldNode->rightChild)) {
		if(oldNode->isRightChild())
			oldNode->parent->rightChild = nil;
		else
			oldNode->parent->leftChild = nil;

		if(oldNode == treeRoot)
			treeRoot = nil;

		return;
	} else if(!isNil(oldNode->leftChild) &&
			!isNil(oldNode->rightChild)) {
		EventNode *iop = inorderPredecessor(oldNode);
		del(iop);// This has only one child, okay!

		iop->leftChild = oldNode->leftChild;
		iop->rightChild = oldNode->rightChild;
		replaceRoot(oldNode, iop);
		iop->color = oldNode->color;

		return;
	}

	/* Here we can assume that oldNode has only one child, and
	   hence, simplify the deletion process. */

	EventNode *nOnlyChild = (oldNode->leftChild != nil) ?
			oldNode->leftChild : oldNode->rightChild;

	replaceRoot(oldNode, nOnlyChild);

	if(oldNode->color == kBlack) {
		if(nOnlyChild->color == kRed) {
			nOnlyChild->color = kBlack;
		} else {
			fixDeletor(nOnlyChild);
		}
	}
}

/**
 * Essentially adds a new event-node in this sorted, updating the left-most
 * and right-most nodes.
 *
 * @version 1.0
 * @since Silcos 3.02
 * @author Shukant Pal
 */
void NodeSorter::put(EventNode *newElem)
{
	if(treeRoot == null) {
		treeRoot = newElem;
		newElem->color = kBlack;
		newElem->parent = nil;
		newElem->rightChild = nil;
		newElem->leftChild = nil;
		nodeCount = 1;

		setMostRecent(newElem);
		setMostLate(newElem);

		return;
	}

	getInitialLeaf(newElem, treeRoot);
	repairTree(newElem);

	if(!checkMostRecent(newElem))
		checkMostLate(newElem);

	++(nodeCount);
}

/**
 * Sets the given "new" node as a leaf at the appropriate position
 * in the tree, without re-balancing the tree/doing any rotations.
 *
 * @param newLeaf - The "new" node added to this sorted tree
 * @param treeRoot - The tree root of this sorted tree
 * @return - The parent of the "new" node after being inserted as
 * 			a leaf.
 * @version 1.0
 * @since Silcos 3.02
 * @author Shukant Pal
 */
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
bool NodeSorter::checkMostRecent(EventNode *newLeaf)
{
	if(newLeaf->overlapRange[0] <
			getMostRecent()->overlapRange[0]) {
		setMostRecent(newLeaf);
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
bool NodeSorter::checkMostLate(EventNode *newLeaf)
{
	if(newLeaf->overlapRange[0] >
			getMostLate()->overlapRange[0]) {
		setMostLate(newLeaf);
		return (true);
	} else {
		return (false);
	}
}

/**
 * Finds the node in this sorted-subtree given, so that the given range
 * can be used to constrain that node, e.g. by adding a trigger with this
 * range. It searches conditionally in both child paths of this subtree
 * using recursion.
 *
 * @param subtree - The subtree/node-root in which the required node is to
 * 				be found. This argument is used for recursion, and should be
 * 				usually this->treeRoot.
 * @param rangeStart - The start of the constraining time-range.
 * @param rangeEnd - The end of the constraining time-range.
 * @return - An event node that can be constrained with the given time-range
 * 			if found within this subtree; (@code strong_null) is used to
 * 			indicate that no such node exists.
 * @version 1.0
 * @since Silcos 3.05
 * @author Shukant Pal
 */
EventNode *NodeSorter::findFor(EventNode *subtree, Timestamp rangeStart,
		Timestamp rangeEnd)
{
	if(isNil(subtree))
		return (strong_null);

	if(subtree->isHoldable(rangeStart, rangeEnd))
		return (subtree);

	if(!isNil(subtree->leftChild) &&
			rangeStart < subtree->leftChild->overlapRange[1]) {

		EventNode *lfn = findFor(subtree->leftChild, rangeStart, rangeEnd);
		if(lfn != null)
			return (lfn);
	}

	if(!isNil(subtree->rightChild) &&
			rangeEnd > subtree->rightChild->overlapRange[0]) {

		EventNode *rfn = findFor(subtree->rightChild, rangeStart, rangeEnd);
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
EventNode *NodeSorter::leanLeft(EventNode *localRoot)
{
	EventNode *newRoot = localRoot->rightChild;
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
EventNode *NodeSorter::leanRight(EventNode *localRoot)
{
	EventNode *newRoot = localRoot->leftChild;
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
 * @since Silcos 3.02
 * @author Shukant Pal
 */
void NodeSorter::repairTree(EventNode *newLeaf)
{
	EventNode *lParent = newLeaf->parent, *lUncle, *gParent;

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
 * @author Shukant Pal
 */
void NodeSorter::fixDeletor(EventNode *nOnlyChild)
{
	if(isNil(nOnlyChild->parent))
		return;

	EventNode *ncSibling = nOnlyChild->getSibling();

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
