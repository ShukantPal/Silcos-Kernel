/**
 * File: AVLTree.cpp
 * Module: ExecutionManager (@kernel.silcos.excmgr)
 *
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
#include <Utils/AVLTree.hpp>

#include <Math.hpp>
#include <KERNEL.h>

static AVLLinker *AVLRotateRight(AVLLinker *node)
{
	AVLLinker *x = node->leftChild;
	AVLLinker *y = x->rightChild;

	x->rightChild  = node;
	node->leftChild = y;

	node->depth = Math::max(AVLLinker::getHeight(node->leftChild), AVLLinker::getHeight(node->rightChild)) + 1;
	x->depth = Math::max(AVLLinker::getHeight(x->leftChild), AVLLinker::getHeight(x->rightChild)) + 1;

	return (x);
}

static AVLLinker *AVLRotateLeft(AVLLinker *newNode)
{
	AVLLinker *y = newNode->rightChild;
	AVLLinker *x = y->leftChild;

	y->leftChild = newNode;
	newNode->rightChild = x;

	newNode->depth = Math::max(AVLLinker::getHeight(newNode->leftChild), AVLLinker::getHeight(newNode->rightChild)) + 1;
	y->depth = Math::max(AVLLinker::getHeight(y->leftChild), AVLLinker::getHeight(y->rightChild)) + 1;

	return (y);
}

/**
 * InsertNodeInBrach() -
 *
 * Summary:
 * This function is used to insert a node into a subtree (branch), recursively. It
 * will add the after reaching the proper leaf and then does the required rotations
 * at each level.
 *
 * Args:
 * newNode - Node to be inserted
 * rootNode - Subtree into which it should be inserted
 *
 * @Version 1
 * @Since Circuit 2.01
 */
static AVLNode *InsertNodeInBranch(AVLNode *newNode, unsigned long *insertionStatus, AVLNode *rootNode)
{
	if(rootNode == NULL)
	{
		*insertionStatus = NODE_INSERTED;
		return (newNode);
	}

	if(newNode->sortValue < rootNode->sortValue)
	{
		rootNode->leftChild = InsertNodeInBranch(newNode, insertionStatus, rootNode->leftChild);
	}
	else if(newNode->sortValue > rootNode->sortValue)
	{
		rootNode->rightChild = InsertNodeInBranch(newNode, insertionStatus, rootNode->rightChild);
	}
	else
	{
		*insertionStatus = NODE_FOUND;
		return (rootNode); // Indicate that matching root is found.
	}

	AVLNode *leftChild = rootNode->leftChild;
	AVLNode *rightChild = rootNode->rightChild;
	unsigned long nodeIndicator = newNode->sortValue;

	rootNode->depth = Max(AVLNode::getHeight(leftChild), AVLNode::getHeight(rightChild)) + 1;
	long nodeBalance = AVLNode::getBalance(rootNode);

	if(nodeBalance > 1)
	{
		if(nodeIndicator < leftChild->sortValue){
			return (AVLNode*) (AVLRotateRight((AVLLinker*) rootNode));
		} else if(nodeIndicator > leftChild->sortValue){
			rootNode->leftChild = (AVLNode*) AVLRotateLeft((AVLLinker*) leftChild);
			return (AVLNode*) (AVLRotateRight((AVLLinker*) rootNode));
		}
	}
	else if(nodeBalance < -1)
	{
		if(nodeIndicator > rightChild->sortValue)
		{
			return (AVLNode*) AVLRotateLeft((AVLLinker*) rootNode);
		}
		else if(nodeIndicator < rightChild->sortValue)
		{
			rootNode->rightChild = (AVLNode*) AVLRotateRight((AVLLinker*) rightChild);
			return (AVLNode*) AVLRotateLeft((AVLLinker*) rootNode);
		}
	}

	return (rootNode); // newNode could not be inserted
}

AVLNode *AVLSearch_(SIZE Indicator, AVLNode *newNode)
{
	if(newNode == NULL)
		return (NULL);

	if(Indicator < newNode->sortValue)
		return (AVLSearch_(Indicator, newNode -> leftChild));
	else if(Indicator > newNode->sortValue)
		return (AVLSearch_(Indicator, newNode -> rightChild));
	else
		return (newNode);
}

decl_c AVLNode *MinValueNode(AVLNode *newNode)
{
	AVLNode *Current = newNode;
	if(Current == NULL) return NULL;

	while(Current -> leftChild != NULL)
		Current = Current -> leftChild;

	return (Current);
}

decl_c AVLNode *MaxValueNode(AVLNode *newNode)
{
	AVLNode *Current = newNode;
	if(Current == NULL) return (NULL);

	while(Current-> rightChild != NULL)
		Current = Current -> rightChild;

	return (Current);
}

/**
 * DeleteNodeInBranch() - 
 *
 * Summary:
 * This function is used for deleting the node in a subtree. It does so by first
 * locating the node and then doing the required operations.
 *
 * It uses the node's Indicator instead of its pointer.
 *
 * Args:
 * Indicator - of the node to delete
 * rootNode - Subtree from which it is to be removed
 *
 * @Version 2.1
 * @Since Circuit 2.01
 */
static AVLNode *DeleteNodeInBranch(SIZE Indicator, AVLNode **deletedNode, AVLNode *rootNode){
	if(rootNode == NULL)
		return (rootNode);

	if(Indicator < rootNode->sortValue)
	{
		rootNode->leftChild= DeleteNodeInBranch(Indicator, deletedNode, rootNode->leftChild);
	}
	else if(Indicator > rootNode->sortValue)
	{
		rootNode->rightChild = DeleteNodeInBranch(Indicator, deletedNode, rootNode->rightChild);
	}
	else
	{
		*deletedNode = rootNode;
		AVLNode *childNode; // This node is to be deleted (freed).
		if((rootNode->leftChild == NULL) || (rootNode->rightChild == NULL))
		{
			childNode = (rootNode->leftChild) ? rootNode->leftChild : rootNode->rightChild;

			if(childNode == NULL)
			{
				childNode = rootNode;
				rootNode = NULL;
			}
			else
			{
				rootNode = childNode;
				childNode = rootNode;
			}
		}
		else
		{
			AVLNode *rightHalver = MinValueNode(rootNode->rightChild);
			AVLNode *newRoot = DeleteNodeInBranch(rightHalver->sortValue, deletedNode, rootNode->rightChild);
			rightHalver->rightChild = newRoot;
			rightHalver->leftChild = rootNode->leftChild;
			rootNode = rightHalver;
		}
	}

	if(rootNode == NULL)
		return (NULL);

	rootNode->depth = Max(AVLNode::getHeight(rootNode->leftChild), AVLNode::getHeight(rootNode->rightChild)) + 1;
	SSIZE_T nodeBalance = AVLNode::getBalance(rootNode);

	if(nodeBalance > 1)
	{
		if(AVLNode::getBalance(rootNode->leftChild) >= 0)
		{
			return (AVLNode*) AVLRotateRight((AVLLinker*) rootNode);
		}
		else if(AVLNode::getBalance(rootNode->leftChild) < 0)
		{
			rootNode->leftChild = (AVLNode*) AVLRotateLeft((AVLLinker*) rootNode->leftChild);
			return (AVLNode*) AVLRotateRight((AVLLinker*) rootNode);
		}
	}
	else if(nodeBalance < -1)
	{
		if(AVLNode::getBalance(rootNode->rightChild) <= 0)
		{
			return (AVLNode*) AVLRotateLeft((AVLLinker*) rootNode);
		}
		else if(AVLNode::getBalance(rootNode->rightChild) > 0)
		{
			rootNode->rightChild = (AVLNode*) AVLRotateRight((AVLLinker*) rootNode->rightChild);
			return (AVLNode*) AVLRotateLeft((AVLLinker*) rootNode);
		}
	}

	return (rootNode);
}

decl_c AVLNode *AVLFindGTE(unsigned long nodeValue, AVLTree *tree)
{
	AVLNode *curNode = tree->treeRoot;
	AVLNode *closestNode = NULL;

	unsigned long curValue;
	unsigned long curDiff = 0;
	unsigned long lastDiff = 0xFFFFFFFF;

	while(curNode != NULL)
	{
		curValue = curNode->sortValue;
		curDiff = curValue - nodeValue;
		if(curDiff < lastDiff)
		{
			closestNode = curNode;
			if(curDiff == 0)
				break;
		}

		if(nodeValue < curValue)
			curNode = curNode->rightChild;
		else
			curNode = curNode->leftChild;
		/* Equal-to case already resolved */
	}

	return (closestNode);
}

decl_c unsigned long AVLInsert(AVLNode *newNode, AVLTree *Tree)
{
	unsigned long insertionStatus;

	Tree ->treeRoot = InsertNodeInBranch(newNode, &insertionStatus, Tree -> treeRoot);
	if(insertionStatus == NODE_INSERTED){
		++(Tree->nodeCount);
	}

	return (insertionStatus);
}

decl_c AVLNode *AVLDelete(SIZE Indicator, AVLTree *nodeTree)
{
	AVLNode *deletedNode = NULL;

	nodeTree->treeRoot = DeleteNodeInBranch(Indicator, &deletedNode, nodeTree->treeRoot);
	if(deletedNode != NULL){
		--(nodeTree->nodeCount);
	}

	return (deletedNode);
}

decl_c AVLNode *AVLSearch(SIZE Indicator, AVLTree *Tree)
{
	return AVLSearch_(Indicator, Tree -> treeRoot);
}
