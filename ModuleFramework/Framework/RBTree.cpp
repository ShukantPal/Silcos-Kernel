/**
 * File: RBTree.cpp
 *
 * (Implementation taken from geeksforgeeks.org)
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
#include <Utils/RBTree.hpp>
#include <KERNEL.h>

extern ObjectInfo *tRBNode;

RBTree::RBTree() : BinaryTree()
{
	nil = (BinaryNode*) new(tRBNode) RBNode();
	((RBNode*) nil)->setColour(RB_BLACK);
	treeRoot = nil;
}

RBTree::~RBTree()
{
	kobj_free((kobj*) nil, tRBNode);
}

/**
 * Adds the key/value pair in this tree, allocating a new node for the same.
 *
 * @param key
 * @param value
 * @return
 */
bool RBTree::insert(unsigned long key, void *value)
{
	RBNode *newNode = new(tRBNode) RBNode(key, value, (RBNode*) nil);

	if(!BinaryTree::insert(*newNode, *this)) {
		kobj_free((kobj*) newNode, tRBNode);
		return (false);
	}

	fixInsert(newNode);
	return (true);
}

/**
 * Deletes the node identified with the given key, saving and returning its
 * value. The node is actually first searched in this process - which could
 * cause more latency than expected. In addition, aftermath balancing is
 * also done!
 *
 * @param key - the node with this key will be removed.
 * @return - the value of node removed; strong_null, if the node couldn't be
 * 			found.
 */
void *RBTree::remove(unsigned long key)
{
	RBNode *tNode = (RBNode*) search(key, *this), *tnChild = strong_null;

	if(isNil(tNode) || !tNode) {
		return (strong_null);
	}

	void *value_found = tNode->val();

	if(isNil(tNode->getRightChild())) {
		tnChild = (RBNode*) tNode->getLeftChild();
	} else if(isNil(tNode->getLeftChild())) {
		tnChild = (RBNode*) tNode->getRightChild();
	} else {
		RBNode *tnr = (RBNode*) tNode->getRightChild();
			
		while(!isNil(tnr->getLeftChild())) {
			tnr = (RBNode*) tnr->getLeftChild();
		}
			
		tNode->searchKey = tnr->searchKey;
		tNode->associatedValue = tnr->associatedValue;
		tnChild = (RBNode*) tnr->getRightChild();
	}

	tnChild = isNil(tnChild) ? tNode :
			(RBNode*) &replaceChild(*tNode, *tnChild);
				
	if(tNode->isBlack()) {
		fixRemoval(tnChild);
	}
		
	if(tNode == tnChild) {
		replaceChild(*tNode, *nil);
	}

	kobj_free((kobj*) tNode, tRBNode);
	return (value_found);
}

/*
 * It is used for swapping the color of two internal nodes, during violation
 * fixing (rotation).
 */
static inline void SwapColour(RBColor *arg0, RBColor *arg1)
{
	volatile RBColor __cache_of_0 = *arg0;
	*arg0 = *arg1;
	*arg1 = __cache_of_0;
}

/**
 * Rotates the subtree at tNode towards the left - making the right-child the
 * new root. This also makes tNode the left-child of the new root, whereas the
 * old left-subtree of the new-root (old right-child of tNode) becomes the
 * right-child of tNode. Confusing, right?
 *
 * @param tNode - the subtree root to be rotated
 * @return - the new root of the rotated subtree
 */
RBNode& RBTree::rotateLeft(RBNode& tNode)
{
	RBNode& edgeNode =* (RBNode*) tNode.getRightChild();
	replaceChild(tNode, edgeNode);

	assignRight(tNode, *edgeNode.getLeftChild());
	assignLeft(edgeNode, tNode);

	SwapColour(&tNode.colorMode, &edgeNode.colorMode);
	return (edgeNode);
}

/**
 * Rotates the subtree at tNode towards the right - making the left-child the
 * new root. This also makes tNode the right-child of the new root, whereas the
 * old right-subtree of the new-root (old left-child of tNode) becomes the
 * left-child of tNode. Confusing, right?
 *
 * @param tNode - the subtree root to be rotated
 * @return - the new root of the rotated subtree
 */
RBNode& RBTree::rotateRight(RBNode& tNode)
{
	RBNode& edgeNode =* (RBNode*) tNode.getLeftChild();
	replaceChild(tNode, edgeNode);

	assignLeft(tNode, *edgeNode.getRightChild());
	assignRight(edgeNode, tNode);

	SwapColour(&tNode.colorMode, &edgeNode.colorMode);
	return (edgeNode);
}

/**
 * Ensures the properties of this red-black tree are not violated after
 * doing an insertion. It tries re-coloring and then rotations to maintain
 * those properties, keeping the tree balanced.
 *
 * This implementation uses iteration, to save resources.
 *
 * @param tNode - the newly inserted node
 */
void RBTree::fixInsert(RBNode *tNode)
{
	RBNode *tnParent, *tnUncle;
	
	while(!tNode->isBlack() && !tNode->isParentBlack()) {
		tnParent = tNode->getColouredParent();
		tnUncle = (RBNode*) tnParent->sibling();

		if(tnUncle->isRed()) {
			tnUncle->setColour(RB_BLACK);
			tnParent->setColour(RB_BLACK);
			tnParent->getColouredParent()->setColour(RB_RED);
			tNode = tnParent->getColouredParent();
		} else {
			if(tNode->isLeftChild() != tnParent->isLeftChild()) {
				tnParent = &rotateReverse(*tNode);
			}
			tNode = &rotateReverse(*tnParent);
		}
	}

	if(isNil(tNode->getParent())) {
		tNode->setColour(RB_BLACK);
	}
}

/**
 * Ensures the properties of this red-black tree aren't violated after doing a
 * deletion operation - it tries re-coloring and then rotations to keep this
 * tree balanced.
 *
 * @param tNode - see remove() for details
 */
void RBTree::fixRemoval(RBNode *tNode)
{
	RBNode *tnSibling;

	while(tNode->isBlack() && !isNil(tNode->getParent())) {
		tnSibling = (RBNode*) tNode->sibling();

		if(tnSibling->isRed()) {
			rotateReverse(*tnSibling);
			tnSibling = (RBNode*) tNode->sibling();
		}

		if(((RBNode*) tnSibling->getLeftChild())->isBlack() &&
				((RBNode*)tnSibling->getRightChild())->isBlack()) {
			tnSibling->setColour(RB_RED);
			tNode = (RBNode*) tNode->getParent();
		} else {
			if(tnSibling->isLeftChild() &&
					!((RBNode*)tnSibling->getLeftChild())->isRed()) {
				tnSibling = (RBNode*) &rotateLeft(*tnSibling);
			} else if(tnSibling->isRightChild() &&
					!((RBNode*)tnSibling->getRightChild())->isRed()) {
				tnSibling = (RBNode*) &rotateRight(*tnSibling);
			}

			rotateReverse(*tnSibling);
			tNode = (RBNode*) tnSibling->getParent()->sibling();
		}
	}

	tNode->setColour(RB_BLACK);
}
