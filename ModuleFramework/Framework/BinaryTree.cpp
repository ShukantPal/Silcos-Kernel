/**
 * @file BinaryTree.cpp
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
#include <Utils/BinaryTree.hpp>
#include <KERNEL.h>

// Obsolete - used for looped iteration over a tree
enum ChildSide {
	BST_LEFT,
	BST_RIGHT
};

/**
 * Constructs an empty <tt>BinaryTree</tt> without any <tt>nil</tt>
 * object.
 */
BinaryTree::BinaryTree()
{
	treeRoot = NULL;
	nil = NULL;
}

/**
 * TODO: Create destructors for binary trees.
 */
BinaryTree::~BinaryTree()
{

}

/**
 * Performs a BST-insert operation, placing the node at a leaf
 * position, unless another node holds the same key.
 *
 * @param node - Reference to allocated node object to insert
 * @param bst - <tt>BinaryTree</tt> container to hold the node
 * @return whether the node was inserted
 */
bool BinaryTree::insert(BinaryNode& node, BinaryTree& bst)
{
	if(bst.isNil(bst.treeRoot)) {
		bst.treeRoot = &node;
		return (true);
	}

	unsigned long nkey = node.key();
	BinaryNode *parent = NULL;
	BinaryNode *leaf = bst.treeRoot;

	while(!bst.isNil(leaf)) {
		parent = leaf;

		if(nkey < parent->key()) {
			leaf = parent->getLeftChild();
		} else if(nkey > parent->key()) {
			leaf = parent->getRightChild();
		} else {
			parent->associatedValue = node.val();
			return (false);
		}
	}

	if(nkey < parent->key()) {
		parent->leftChild = &node;
	} else {
		parent->rightChild = &node;
	}

	node.directParent = parent;

	return (true);
}

/**
 * Returns the value of the node holding a key (less than or equal to
 * <tt>key</tt>) closest to the given key. null is returned
 * on finding no valid node, or if the nodal value is null.
 *
 * @param key - the reference key to compare with
 */
void* BinaryTree::getLowerBoundFor(unsigned long key)
{
	BinaryNode *tNode = treeRoot;
	BinaryNode *closestNode = nil;

	while(!isNil(tNode)) {
		if(tNode->key() == key) {
			return (tNode->val());
		} else if(key < tNode->key()) {
			tNode = tNode->getLeftChild();
		} else {
			if(isNil(closestNode) || key - closestNode->key()
					> key - tNode->key()) {
				closestNode = tNode;
			}
			
			tNode = tNode->getRightChild();
		}
	}

	return (!isNil(closestNode)) ? closestNode->val() : NULL;
}

/**
 * Returns the value of the node holding a key (greater than or equal
 * to <tt>key</tt>) closest of the given key. null is returned
 * on finding no valid node, or if the nodal value is null.
 *
 * @param key - the reference key to compare with
 */
void* BinaryTree::getUpperBoundFor(unsigned long key)
{
	BinaryNode* tNode = treeRoot;
	BinaryNode* closestNode = nil;

	while(!isNil(tNode)) {
		if(tNode->key() == key) {
			return (tNode->val());
		} else if(key < tNode->key()) {
			if(isNil(closestNode) || closestNode->key() - key
					> tNode->key() - key) {
				closestNode = tNode;
			}
			tNode = tNode->getLeftChild();
		} else {
			tNode = tNode->getRightChild();
		}
	}

	return (!isNil(closestNode)) ? closestNode->val() : NULL;
}

/**
 * Returns the value of the node holding a key closest the given
 * key. null is returned if only one node is present, or if the
 * nodal value itself is null.
 *
 * @param key - the reference key to compare with
 */
void* BinaryTree::getClosestOf(unsigned long key)
{
	BinaryNode* tNode = treeRoot;
	BinaryNode* closestNode = nil;
	bool isClosestNodeLower = false;// no meaning when closestNode is NULL

	while(!isNil(tNode)) {
		if(tNode->key() == key) {
			return (tNode->val());
		} else if(key < tNode->key()) {
			if(isNil(closestNode)) {
				if(isClosestNodeLower && tNode->key() - key
						< key - closestNode->key()) {
					closestNode = tNode;
					isClosestNodeLower = false;
				} else if(tNode->key() - key
						< closestNode->key() - key) {
					closestNode = tNode;
					isClosestNodeLower = false;
				}
			}

			tNode = tNode->getLeftChild();
		} else {
			if(isNil(closestNode)) {
				if(isClosestNodeLower && key - tNode->key()
						< key - closestNode->key()) {
					closestNode = tNode;
					isClosestNodeLower = true;
				} else if(key - tNode->key()
						< closestNode->key() - key) {
					closestNode = tNode;
					isClosestNodeLower = true;
				}
			}
		}
	}

	return (!isNil(closestNode)) ? (closestNode->val()) : NULL;
}

/**
 * Returns the value of the node in this tree, having the maximum
 * value compared to all others.
 */
void *BinaryTree::getMaximum()
{
	BinaryNode *tNode = treeRoot;

	while(!isNil(tNode->getRightChild())) {
		tNode = tNode->getRightChild();
	}

	return (isNil(tNode)) ? NULL : tNode->val();
}

/**
 * Returns the node holding the given key in the given tree. If no
 * such node exists, null is returned.
 *
 * @param key - the key of node being searched
 * @param bst - the tree in which the node is stored
 */
BinaryNode *BinaryTree::search(unsigned long key, BinaryTree& bst)
{
	BinaryNode *tnode = bst.treeRoot;

	while(!bst.isNil(tnode)) {
		if(key < tnode->key()) {
			tnode = tnode->getLeftChild();
		} else if(key > tnode->key()) {
			tnode = tnode->getRightChild();
		} else {
			return (tnode);
		}
	}

	return (null);
}
/*
String& BinaryTree::toString()
{
	return* new String("@com.silcos.circuit.mdfrwk::BinaryTree");
}
*/
