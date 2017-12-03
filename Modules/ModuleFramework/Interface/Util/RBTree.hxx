/**
 * File: RBTree.hxx
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
#ifndef MODULES_MODULEFRAMEWORK_RBTREE_HXX_
#define MODULES_MODULEFRAMEWORK_RBTREE_HXX_

#include "BinaryTree.hxx"
#include <TYPE.h>
include_kobject(tRBNode)

enum RBColor
{
	RB_RED = 1,
	RB_BLACK = 2
};

/**
 * Struct: RBNode
 *
 * Summary:
 * RBNode is a variant of the BinaryNode which is used in all the red-black
 * tree variants and includes a color-attribute (@See RBColor).
 *
 * Version: 1.1
 * Since: MDFRWK 1.0
 * Author: Shukant Pal
 */
struct RBNode : public BinaryNode
{
	RBColor colorMode;/* Red/Black color indicator */
	friend class RBTree;

	/*
	 * This constructor is used when parent & children must be equal to
	 * this node only (for nil).
	 */
	RBNode() : BinaryNode()
	{
		this->colorMode = RB_RED;
	}

	/*
	 * This constructor is used when parent & children must be a null
	 * pointer (not used for RBTree).
	 */
	RBNode(long uniqueKey, void *mappedValue) : BinaryNode(uniqueKey, mappedValue, 0,0,0)
	{
		colorMode = RB_RED;
	}

	/*
	 * This constructor is used when parent & children must point to a
	 * sentinel nil node.
	 */
	RBNode(long key, void *value, RBNode *nil) : BinaryNode(key, value, nil, nil, nil)
	{
		colorMode = RB_RED;
	}

	inline bool isBlack(){ return (colorMode == RB_BLACK); }
	inline bool isRed(){ return (colorMode == RB_RED); }
	inline RBColor getColour(){ return colorMode; }
	inline void setColour(RBColor rc){ colorMode = rc; }
	inline RBNode *getColouredParent(){ return (RBNode*) directParent; }
	inline RBColor getParentColour(){ return getColouredParent()->getColour(); }
	inline bool isParentBlack(){ return getParentColour() == RB_BLACK; }
	inline bool isParentRed(){ return getParentColour() == RB_RED; }
};

/**
 * Class: RBTree
 *
 * Summary:
 * Red-black tree is a self-balancing variant of binary-search trees, where
 * each node has a extra attribute - colour (@See RBNode). The leaf-nodes
 * of this don't contain data (@See RBTree::nil) and a sentinel node performs
 * its role. It provides a O(log n) search time and has a least possible tree
 * height.
 *
 * The red-black tree satisfies the following constraints -
 *
 * 1. Each node is either red or black
 *
 * 2. The root is always black (omitted before any insertion)
 *
 * 3. All leaves (nil) are black
 *
 * 4. If a node is red, then both its children will be black
 *
 * 5. Every path from a given node to any of its descendant nil nodes contain
 * the same number of black-nodes.
 *
 * No path is more than twice as long as any other path in tree.
 *
 * This tree should be used when insertion/deletion operations are comparable
 * in number to the search operations & the tree is not huge.
 *
 * Version: 1.2
 * Since: MDFRWK 1.0
 * Author: Shukant Pal
 * See: RBNode, BinaryNode, BinaryTree
 */
class RBTree final : public BinaryTree
{
public:
	RBTree();
	~RBTree();
	bool insert(unsigned long key, void * value);
	void* remove(unsigned long anyKey);

	void _dub();
protected:
	RBNode& rotateLeft(RBNode& tNode);
	RBNode& rotateRight(RBNode& tNode);

	inline RBNode& rotateReverse(RBNode& tNode)
	{
		if(tNode.isLeftChild())
			return rotateRight(*tNode.getColouredParent());
		else // tNode.isRightChild()
			return rotateLeft(*tNode.getColouredParent());
	}

	void fixInsert(RBNode *tNode);
	void fixRemoval(RBNode *tNode);
};

#endif/* ModuleFramework/RBTree.hxx*/
