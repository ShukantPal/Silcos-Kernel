/**
 * File: RBTree.hpp
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
#ifndef _MDFRWK_UTIL_RBTREE_HPP_
#define _MDFRWK_UTIL_RBTREE_HPP_

#include <TYPE.h>
#include "../Utils/BinaryTree.hpp"
include_kobject(tRBNode)
include_kobject(tRBTree)

enum RBColor
{
	RB_RED = 1,
	RB_BLACK = 2
};

/**
 * The red-black node is a variant of binary-search nodes, and has a
 * defined color (red/black), used for balancing the red-black
 * tree.
 *
 * This is not exposed to users of the red-black tree, instead it is
 * confined internally.
 */
struct RBNode : public BinaryNode
{
	RBColor colorMode;
	friend class RBTree;

	/* This constructor is used for initializing the "nil" node - whose
	   children and parent is "this" itself. */
	RBNode() : BinaryNode() {
		this->colorMode = RB_RED;
	}

	/* This constructor initializes normal leaves - pointing the children
	   & parent to the "nil" node. */
	RBNode(long key, void *value, RBNode *nil)
			: BinaryNode(key, value, nil, nil, nil) {
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
 * The red-black tree is self-balancing binary-search-tree, with each node
 * having a defined colour, particularly used for deferring rotations. It uses
 * "nil" nodes instead of null-pointers for leaf-nodes, which have some
 * pre-defined properties.
 *
 * This tree provides a O(log(n)) search-time and has faster amortized deletion
 * costs compared to the AVLTree. It in turn has a less-balanced nature than
 * the AVL tree.
 *
 * You should use this, when the tree will be used for insertion/deletion
 * heavily w.r.t searches.
 *
 * @version 1.2
 * @since Silcos 3.02
 * @author: Shukant Pal
 * @see RBNode, BinaryNode, BinaryTree
 */
class RBTree final : public BinaryTree
{
public:
	RBTree();
	~RBTree();
	bool insert(unsigned long key, void * value);
	void* remove(unsigned long anyKey);
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

#endif/* Utils/RBTree.hpp */
