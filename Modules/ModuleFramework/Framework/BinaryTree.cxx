/**
 * File: BST.cxx
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
#include <Util/BinaryTree.hxx>
#include <KERNEL.h>
//using namespace Util;

enum ChildSide
{
	BST_LEFT,
	BST_RIGHT
};

BinaryTree::BinaryTree()
{
	treeRoot = NULL;
	nil = NULL;
}

BinaryTree::~BinaryTree()
{

}

/**
 * Function: BinaryTree::insert
 *
 * Summary:
 * BST-insert operation; inserts the given node into the root's data tree, unless
 * another node of the same key is found.
 *
 * Args:
 * BinaryNode& node - Node to be inserted
 *
 * Returns:
 * true, if the node was inserted; false, if another node of the same key was found
 * & this node couldn't be inserted.
 *
 * Since: MDFRWK 1.0
 * Author: Shukant Pal
 */
bool BinaryTree::insert(
		BinaryNode& node,
		BinaryTree& bst
){
	if(bst.isNil(bst.treeRoot)){
		bst.treeRoot = &node;
	} else {
		unsigned long nkey = node.key();
		BinaryNode *parent = NULL;
		BinaryNode *leaf = bst.treeRoot;
		while(!bst.isNil(leaf)){
			parent = leaf;

			if(nkey < parent->key()){
				leaf = parent->getLeftChild();
			} else if(nkey > parent->key()){
				leaf = parent->getRightChild();
			} else {
				return (false);
			}
		}

		if(nkey < parent->key()){
			parent->leftChild = &node;
		} else {
			parent->rightChild = &node;
		}

		node.directParent = parent;
	}
	return (true);
}

/**
 * Function: BinaryTree::search
 *
 * Summary:
 * Searches for a node with the given key, and returns its storing node. It
 * assumes the binary-tree under root is sorted.
 *
 * Args:
 * unsigned long key - Key to search in the tree
 * BinaryNode& root - Root node to search from
 *
 * Returns:
 * Pointer to the node containing the key given.
 *
 * Since: MDFRWK 1.0
 * Author: Shukant Pal
 */
BinaryNode *BinaryTree::search(
		unsigned long key,
		BinaryTree& bst
){
	BinaryNode *tnode = bst.treeRoot;
	while(!bst.isNil(tnode)){
		if(key < tnode->key())
			tnode = tnode->getLeftChild();
		else if(key > tnode->key())
			tnode = tnode->getRightChild();
		else
			return (tnode);
	}

	return (NULL);
}
/*
String& BinaryTree::toString()
{
	return* new String("@com.silcos.circuit.mdfrwk::BinaryTree");
}
*/
