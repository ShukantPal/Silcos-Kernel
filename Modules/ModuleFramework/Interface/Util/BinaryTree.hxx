/**
 * File: BST.hxx
 *
 * Summary:
 * Defines how abstract binary-trees are interfaced with and how their nodes are
 * implemented.
 *
 * Struct:
 * BinaryNode - Node of a binary-search tree
 * 
 * Class:
 * BinaryTree - Abstraction of a binary-search tree (with pure-virtual functions)
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
#ifndef MODULES_MODULEFRAMEWORK_INTERFACE_BINARYTREE_HXX_
#define MODULES_MODULEFRAMEWORK_INTERFACE_BINARYTREE_HXX_

#include "Object.hxx"
#include "String.hxx"

enum RecurseDirection
{
	LEFT_UPWARD,
	RIGHT_UPWARD,
	DOWNWARD_LEFT,
	DOWNWARD_RIGHT
};

/**
 * Struct: BinaryNode
 *
 * Summary:
 * Defines how binary-nodes are organized in a tree with a pointer to the
 * parent of each node. All the members are public as they are not meant
 * to be shared with code external to binary-trees.
 *
 * Functions:
 * isLeftChild - Tells whether is node is the left-child of its parent
 * isRightChild - Tells whether is node is the right-child of its parent
 * getLeftChild - Returns the left-child of this node
 * getRightChild - Returns the right-child of this node
 * getParent - Returns the parent of this node
 * setLeftChild - Links the other node as left-child of this
 * setRightChild - Links the other node as right-child of this
 * getSibling - Returns the other child of the parent
 * replaceWith - Replaces this with another node as the child of its parent
 *
 * Author: Shukant Pal
 */
struct BinaryNode
{
public:
	inline unsigned long key(){ return (searchKey); }
	inline void* val(){ return (associatedValue); }

	BinaryNode *leftChild;/* Left-child of this node */
	BinaryNode *rightChild;/* Right-child of this node */
	BinaryNode *directParent;/* Parent of this node */
	unsigned long searchKey;/* Key, which is unique for this node (in a subtree) */
	void* associatedValue;/* Value associated with key, to retrieve data */

	inline BinaryNode *getLeftChild(){ return (leftChild); }
	inline BinaryNode *getRightChild(){ return (rightChild); }
	inline BinaryNode *getParent(){ return (directParent); }

	inline bool isLeftChild()
	{
		return (this == getParent()->getLeftChild());
	}

	inline bool isRightChild()
	{
		return (this == getParent()->getRightChild());
	}

	/*
	 * This should be used for trees which DON'T USE NIL NODES
	 */
	inline void setLeftChild(BinaryNode *tnode)
	{
		this->leftChild = tnode;
		tnode->directParent = this;
	}

	/*
	 * This should be used for trees which DON'T USE NIL NODES.
	 */
	inline void setRightChild(BinaryNode *tnode)
	{
		this->rightChild = tnode;
		tnode->directParent = this;
	}

	/**
	 * Function: BinaryNode::getSibling
	 *
	 * Summary:
	 * Returns the sibling node of this, e.g. if isRightChild() == TRUE then
	 * getParent()->leftChild() & vice-versa.
	 *
	 * Note: Make sure this.getParent() != NULL
	 *
	 * Author: Shukant Pal
	 */
	inline BinaryNode *getSibling()
	{
		if(isLeftChild())
			return (directParent->rightChild);
		else
			return (directParent->leftChild);
	}

	/**
	 * Function: BinaryNode::replaceWith
	 *
	 * Summary:
	 * Replaces this node, along with its subtree, with the replacer by
	 * resetting the child of the direct-parent. It is used during rotation
	 * and care must be taken to ensure that (getParent() != NULL) is
	 * satisfied. (@See BinaryTree::replace())
	 *
	 * For tree that use this, DON'T USE NIL NODES
	 *
	 * Args:
	 * BinaryNode *replacer - node which will replace this in the upper-tree
	 *
	 * Author: Shukant Pal
	 */
	inline void replaceWith(BinaryNode *replacer)
	{
		if(isLeftChild())
			directParent->leftChild = replacer;
		else
			directParent->rightChild = replacer;

		if(replacer)
			replacer->directParent = this->directParent;
	}

protected:
	BinaryNode()
	{
		this->leftChild =
				this->rightChild =
						this->directParent =
								this;
		this->searchKey = 0;
		this->associatedValue = 0;
	}

	BinaryNode(unsigned long searchKey, void *value, BinaryNode *left, BinaryNode *right, BinaryNode *parent)
	{
		this->leftChild = left;
		this->rightChild = right;
		this->directParent = parent;
		this->searchKey = searchKey;
		this->associatedValue = value;
	}

	friend class BinaryTree;
};

/**
 * Class: BinaryTree
 *
 * Summary:
 * Abstracts a binary tree which allows insertion/deletion/searching.
 *
 * Functions:
 * insert - Inserts a key-value pair in the binary-tree
 * remove - Removes a node with a given key (iff found)
 * get - Returns the value of the node with the assigned key
 * set - Sets the value of the node with the assigned key
 *
 * Author: Shukant Pal
 */
class BinaryTree : public ::Object
{
public:
	virtual ~BinaryTree();
	virtual bool insert(unsigned long key, void *value) = 0;
	virtual void* remove(unsigned long key) = 0;

	/*
	 * Get the value associated with the key, if it was inserted in the
	 * tree; otherwise, if it wasn't inserted, NULL
	 */
	inline void *get(unsigned long key)
	{
		if(treeRoot){
			BinaryNode *tNode = search(key, *this);
			if(tNode != NULL)
				return tNode->val();
		}
		return (NULL);
	}

	/*
	 * Set the value associated with the key, if it was inserted in the
	 * tree, and return true; otherwise, return false.
	 */
	inline bool set(unsigned long key, void *value)
	{
		if(treeRoot){
			BinaryNode *tNode = search(key, *this);
			if(tNode != NULL){
				tNode->associatedValue = value;
				return (true);
			}
		}
		return (false);
	}

	static inline void printInorder(BinaryTree *tree)
	{
		printInorder(tree->treeRoot, tree->nil);
	}

	//virtual String& toString();
protected:
	BinaryNode *nil;/* Nil node for all leaves (initialized by child) */
	BinaryNode *treeRoot;/* Root of the tree, if any */

	inline bool isNil(BinaryNode *node){ return (node == nil); }
	inline bool isNil(BinaryNode& node){ return (&node == nil); }

	inline void assignLeft(
			BinaryNode& tNode,
			BinaryNode& tnChild
	){
		tNode.leftChild = &tnChild;
		if(!isNil(tnChild))
			tnChild.directParent = &tNode;
	}

	inline void assignRight(
			BinaryNode& tNode,
			BinaryNode& tnChild
	){
		tNode.rightChild = &tnChild;
		if(!isNil(tnChild))
			tnChild.directParent = &tNode;
	}

	static bool insert(BinaryNode& node, BinaryTree& bst);
	static void valueOf(long key, BinaryNode& node);
	static BinaryNode *search(unsigned long key, BinaryTree& _this__);

	/**
	 * Function: BinaryTree::replace
	 *
	 * Summary:
	 * Replaces tNode with replacer along with its subtree(@See replaceWith)
	 *
	 * Args:
	 * BinaryNode& tNode - original node to replace
	 * BinaryNode& nNode - node that will replace tNode in the tree
	 *
	 * Author: Shukant Pal
	 */
	inline BinaryNode& replaceChild(
			BinaryNode& tNode,
			BinaryNode& nNode
	){
		if(isNil(tNode.getParent())){
			treeRoot = &nNode;
			nNode.directParent = nil;
		} else {
			if(tNode.isLeftChild())
				assignLeft(*tNode.getParent(), nNode);
			else
				assignRight(*tNode.getParent(), nNode);
		}

		return (nNode);
	}

	static inline void printPair(BinaryNode *atNode)
	{
		Dbg(__leftparen);
		DbgInt(atNode->key());
		Dbg(__comma);
		DbgInt((unsigned long) atNode->val());
		Dbg(__rightparen);
	}

	static inline void printInorder(BinaryNode *node, BinaryNode *nil)
	{
		unsigned long dir = DOWNWARD_LEFT;

		while(TRUE){
			switch(dir)
			{
			case DOWNWARD_LEFT:
				if(node->getLeftChild() != nil)
					node = node->getLeftChild();
				else {
					printPair(node);
					if(node->isLeftChild())
						dir = LEFT_UPWARD;
					else
						dir = RIGHT_UPWARD;
				}
				break;
			case DOWNWARD_RIGHT:
				if(node->getRightChild() != nil)
					node = node->getRightChild();
				dir = DOWNWARD_LEFT;
				break;
			case LEFT_UPWARD:
				node = node->getParent();
				if(node == nil)
					return;
				printPair(node);
				dir = DOWNWARD_RIGHT;
				break;
			case RIGHT_UPWARD:
				node = node->getParent();
				if(node == nil)
					return;
				if(node->isLeftChild())
					dir = LEFT_UPWARD;
				break;
			}
		}
	}

	BinaryTree();
};

#endif /* ModulesFramework/BinaryTree.hxx */
