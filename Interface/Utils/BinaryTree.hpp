/**
 * @file BinaryTree.hpp
 *
 * The BinaryTree is the root of all binary-search-tree variants. The
 * unsorted binary trees aren't used in this kernel, though. This file
 * provides the common interfaces for BST operations allowing code to
 * reuse this to build a new variant.
 *
 * Note that these objects are non-intrusive. That means you can't use
 * your own nodes with them. It should improve performance - as the
 * nodes will be closely packed - especially in static trees.
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
#ifndef MDFRWK_BINARYTREE_HXX_
#define MDFRWK_BINARYTREE_HXX_

#include "../Object.hpp"
#include "String.hxx"

class BinaryNode;
class TreeIterator;
class BinaryTree;

enum RecurseDirection
{
	LEFT_UPWARD,
	RIGHT_UPWARD,
	DOWNWARD_LEFT,
	DOWNWARD_RIGHT
};

/**
 * Back-bone of all binary-tree nodes - having two children and one
 * parent. Along with these structural components - it has a search key
 * and an associated value.
 */
struct BinaryNode
{
public:
	inline unsigned long key(){ return (searchKey); }
	inline void* val(){ return (associatedValue); }

	BinaryNode *leftChild;/* Left-child of this node */
	BinaryNode *rightChild;/* Right-child of this node */

	union {
		BinaryNode *directParent;/* Parent of this node */
		int localData;
	};

	unsigned long searchKey;/* Key, which is unique for this node (in a subtree) */
	void* associatedValue;/* Value associated with key, to retrieve data */

	inline BinaryNode *getLeftChild(){ return (leftChild); }
	inline BinaryNode *getRightChild(){ return (rightChild); }
	inline BinaryNode *getParent(){ return (directParent); }

	inline void assignLeft(BinaryNode *n) {
		leftChild = n;
	}

	inline void assignRight(BinaryNode *n) {
		rightChild = n;
	}

	inline bool isLeftChild() {
		return (this == getParent()->getLeftChild());
	}

	inline bool isRightChild() {
		return (this == getParent()->getRightChild());
	}

	/* Assumes the tree doesn't use "nil" nodes and the child given is not
	   a null-pointer. @see (BinaryTree::assignLeft()) */
	inline void setLeftChild(BinaryNode *tnode)
	{
		this->leftChild = tnode;
		tnode->directParent = this;
	}

	/* Assumes the tree doesn't use "nil" nodes and the child given is not
	   a null-pointer. @see (BinaryTree::assignRight()) */
	inline void setRightChild(BinaryNode *tnode) {
		this->rightChild = tnode;
		tnode->directParent = this;
	}

	/* Returns the sibling node of this, should be obvious. */
	inline BinaryNode *getSibling() {
		if(isLeftChild()) {
			return (directParent->rightChild);
		} else {
			return (directParent->leftChild);
		}
	}

	/* Replaces this whole subtree (.i.e this node along with children) from
	   the parent's subtree with the replacer. */
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

	BinaryNode(unsigned long searchKey, void *value, BinaryNode *left,
			BinaryNode *right, BinaryNode *parent)
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
 * This is the root-class of all BST variants. It already provides various search
 * operations publicly and a few goodies for internal use.
 */
class BinaryTree : public ::Object
{
public:
	virtual ~BinaryTree();
	virtual bool insert(unsigned long key, void *value) = 0;
	virtual void* remove(unsigned long key) = 0;

	/* Get the value associated with the given key; strong_null, to
	   indicate that the node doesn't exist. */
	inline void *get(unsigned long key) {
		if(treeRoot) {
			BinaryNode *tNode = search(key, *this);
			if(tNode != NULL)
				return tNode->val();
		}
		return (strong_null);
	}

	/*
	 * Set the value associated with the key, if it was inserted in the
	 * tree, and return true; otherwise, return false.
	 */
	inline bool set(unsigned long key, void *value) {
		if(treeRoot) {
			BinaryNode *tNode = search(key, *this);
			if(tNode != NULL) {
				tNode->associatedValue = value;
				return (true);
			}
		}
		return (false);
	}

	void* getLowerBoundFor(unsigned long key);
	void* getUpperBoundFor(unsigned long key);
	void* getClosestOf(unsigned long key);
	void* getMaximum();

	//virtual String& toString();
protected:
	BinaryNode *nil;/* Nil node for all leaves (initialized by child) */
	BinaryNode *treeRoot;/* Root of the tree, if any */

	inline bool isNil(BinaryNode *node){ return (node == nil); }
	inline bool isNil(BinaryNode& node){ return (&node == nil); }

	inline void assignLeft(BinaryNode& tNode, BinaryNode& tnChild) {
		tNode.leftChild = &tnChild;
		if(!isNil(tnChild))
			tnChild.directParent = &tNode;
	}

	inline void assignRight(BinaryNode& tNode, BinaryNode& tnChild) {
		tNode.rightChild = &tnChild;
		if(!isNil(tnChild))
			tnChild.directParent = &tNode;
	}

	static bool insert(BinaryNode& node, BinaryTree& bst);
	static void valueOf(long key, BinaryNode& node);
	static BinaryNode *search(unsigned long key, BinaryTree& _this__);

	inline BinaryNode& replaceChild(BinaryNode& tNode, BinaryNode& nNode) {
		if(isNil(tNode.getParent())) {
			treeRoot = &nNode;
			nNode.directParent = nil;
		} else {
			if(tNode.isLeftChild()) {
				assignLeft(*tNode.getParent(), nNode);
			} else {
				assignRight(*tNode.getParent(), nNode);
			}
		}

		return (nNode);
	}

	BinaryTree();
	friend TreeIterator;
};

/**
 * Provides an generic iterator for all binary-search-trees, to view internal
 * organization. The iterator starts from the treeRoot and can be used to
 * go left, right, and even up the tree.
 *
 * You can get/set the value of each node, and also check its key. Setting the
 * key is not allowed. Modifications done with or without this iterator won't
 * stop this object from iterating. The user must know about any external
 * modifications.
 */
class TreeIterator : public Object
{
public:
	TreeIterator(BinaryTree &bst)
		: bst(bst), node(bst.treeRoot) {
	}

	unsigned long getKey() {
		return (node->key());
	}

	void *getValue() {
		return (node->associatedValue);
	}

	bool goBack() {
		if(!bst.isNil(node->directParent)) {
			node = node->directParent;
			return (true);
		 } else {
			 return (false);
		 }
	}

	bool goLeft() {
		if(!bst.isNil(node->leftChild)) {
			node = node->leftChild;
			return (true);
		} else {
			return (false);
		}
	}

	bool goRight() {
		if(!bst.isNil(node->rightChild)) {
			node = node->rightChild;
			return (true);
		} else {
			return (false);
		}
	}

	void setValue(void *newValue) {
		node->associatedValue = newValue;
	}
private:
	BinaryTree &bst;
	BinaryNode *node;
};

#endif /* ModulesFramework/BinaryTree.hxx */
