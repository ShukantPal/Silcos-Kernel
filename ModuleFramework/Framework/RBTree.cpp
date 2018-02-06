/**
 * File: RBTree.cxx
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
 * --------------------------------------------------------------------
 * Algorithm was kindof referenced from geeksfromgeeks.org (indian ya)
 */
#include "../../Interface/Utils/RBTree.hpp"

#include <KERNEL.h>

extern ObjectInfo *tRBNode;

//using namespace Util;

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
 * Function: RBTree::insert
 *
 * Summary:
 * Inserts a node of the given key, value pair and so keeps the rb-tree
 * balanced.
 *
 * Args:
 * unsigned long key - key, by which you can find the value in the tree
 * void *value - a value that client wants to associated with the key
 *
 * Returns:
 * true, if the key was inserted in the tree; false, if it already existed
 * or couldn't be inserted due a internal-error.
 *
 * Author: Shukant Pal
 */
bool RBTree::insert(unsigned long key, void *value)
{
	RBNode *newNode = new(tRBNode) RBNode(key, value, (RBNode*) nil);
	if(!BinaryTree::insert(*newNode, *this))
	{
		Dbg("_nul_");
		kobj_free((kobj*) newNode, tRBNode);
		return (false);
	}
	fixInsert(newNode);
	return (true);
}

/**
 * Function: RBTree::remove
 *
 * Summary:
 * Removes the node with the given key, if it was inserted into the tree and
 * is freed.
 *
 * Args:
 * unsigned long key - key with which the node was inserted
 *
 * Returns:
 * true, if the node was inserted before, and was deleted now; false, if the it
 * didn't exist.
 *
 * Author: Shukant Pal
 */
void* RBTree::remove(unsigned long key)
{
	RBNode *tNode = (RBNode*) search(key, *this), *tnChild = NULL;
	if(isNil(tNode) || !tNode)
	{
		return (NULL);
	}
	else
	{
		void *value_found = tNode->val();

		if(isNil(tNode->getRightChild()))
		{
			tnChild = (RBNode*) tNode->getLeftChild();
		}
		else if(isNil(tNode->getLeftChild()))
		{
			tnChild = (RBNode*) tNode->getRightChild();
		}
		else
		{
			RBNode *tnr = (RBNode*) tNode->getRightChild();
			
			while(!isNil(tnr->getLeftChild()))
			{
				tnr = (RBNode*) tnr->getLeftChild();
			}
			
			tNode->searchKey = tnr->searchKey;
			tNode->associatedValue = tnr->associatedValue;
			tnChild = (RBNode*) tnr->getRightChild();
		}

		tnChild = isNil(tnChild) ? tNode : (RBNode*) &replaceChild(*tNode, *tnChild);
				
		if(tNode->isBlack())
		{
			fixRemoval(tnChild);
		}
		
		if(tNode == tnChild)
		{
			replaceChild(*tNode, *nil);
		}

		kobj_free((kobj*) tNode, tRBNode);
		return (value_found);
	}
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

RBNode& RBTree::rotateLeft(RBNode& tNode)
{
	RBNode& edgeNode =* (RBNode*) tNode.getRightChild();
	replaceChild(tNode, edgeNode);

	assignRight(tNode, *edgeNode.getLeftChild());
	assignLeft(edgeNode, tNode);

	SwapColour(&tNode.colorMode, &edgeNode.colorMode);
	return (edgeNode);
}

RBNode& RBTree::rotateRight(RBNode& tNode)
{
	RBNode& edgeNode =* (RBNode*) tNode.getLeftChild();
	replaceChild(tNode, edgeNode);

	assignLeft(tNode, *edgeNode.getRightChild());
	assignRight(edgeNode, tNode);

	SwapColour(&tNode.colorMode, &edgeNode.colorMode);
	return (edgeNode);
}

void RBTree::fixInsert(RBNode *tNode)
{
	RBNode *tnParent, *tnUncle;
	
	while(!tNode->isBlack() && !tNode->isParentBlack())
	{
		tnParent = tNode->getColouredParent();
		tnUncle = (RBNode*) tnParent->getSibling();

		if(tnUncle->isRed())
		{
			tnUncle->setColour(RB_BLACK);
			tnParent->setColour(RB_BLACK);
			tnParent->getColouredParent()->setColour(RB_RED);
			tNode = tnParent->getColouredParent();
		}
		else
		{ // tnUncle.isBlack()
			if(tNode->isLeftChild() != tnParent->isLeftChild())
			{
				tnParent = &rotateReverse(*tNode);
			}
			tNode = &rotateReverse(*tnParent);
		}
	}

	if(isNil(tNode->getParent()))
	{
		tNode->setColour(RB_BLACK);
	}
}

void RBTree::fixRemoval(RBNode *tNode)
{
	RBNode *tnSibling;
	while(tNode->isBlack() && !isNil(tNode->getParent()))
	{
		tnSibling = (RBNode*) tNode->getSibling();

		if(tnSibling->isRed())
		{
			rotateReverse(*tnSibling);
			tnSibling = (RBNode*) tNode->getSibling();
		}

		if(((RBNode*) tnSibling->getLeftChild())->isBlack() &&
				((RBNode*)tnSibling->getRightChild())->isBlack())
		{
			tnSibling->setColour(RB_RED);
			tNode = (RBNode*) tNode->getParent();
		}
		else
		{
			if(tnSibling->isLeftChild() &&
					!((RBNode*)tnSibling->getLeftChild())->isRed())
			{
				tnSibling = (RBNode*) &rotateLeft(*tnSibling);
			}
			else if(tnSibling->isRightChild() &&
					!((RBNode*)tnSibling->getRightChild())->isRed())
			{
				tnSibling = (RBNode*) &rotateRight(*tnSibling);
			}

			rotateReverse(*tnSibling);
			tNode = (RBNode*) tnSibling->getParent()->getSibling();
		}
	}

	tNode->setColour(RB_BLACK);
}

static void show_it(BinaryNode *f, BinaryNode *n)
{
	if(f == n) return;
	show_it(f->leftChild, n);
	DbgInt(f->key()); Dbg((char*)" ");
	show_it(f->rightChild, n);
}

void RBTree::_dub()
{
	show_it(treeRoot, nil);
}
