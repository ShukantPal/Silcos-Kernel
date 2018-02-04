/**
 * File: AVLTree.hpp
 * Module: ExecutionManager (@kernel.silcos.excmgr)
 *
 * Summary:
 * Provides a C-style data structure of type - binary search tree. It can be
 * used in asm also (we wanted atleast one to be C-compatible). Note that
 * this is not C-source compatible and requires a separate header for usage
 * with *.c files.
 *
 * The static methods in the structs used need not be put in the C header. They
 * are implementation-required, not for external use.
 *
 * Changes:
 * # Moved from KernelHost to the ExecutionManager for usage in organizing
 *   tasks in CFS scheduler & the tickless timer.
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
#ifndef EXCMGR_UTIL_AVL_TREE_H__
#define EXCMGR_UTIL_AVL_TREE_H__

#include <KERNEL.h>

/**
 * Struct: AVLLinker
 *
 * Summary:
 * This is a struct which is sorted by its address. It has no "sortValue"
 * field, and thus cannot be handled by the standard AVL-tree functions. Other
 * functions are provided for them. They are useful for saving memory in
 * nodes that are to be sorted by address, e.g. descriptors which are stored
 * in order in memory itself.
 *
 * It is extensively used in the implementation too.
 *
 * Version: 1.01
 * Since: Circuit 2.03
 * Author: Shukant Pal
 */
struct AVLLinker
{
	AVLLinker *leftChild;
	AVLLinker *rightChild;
	unsigned long depth;

	static inline long getHeight(AVLLinker *ptr)
	{
		return (ptr) ? (ptr->depth) : -1;
	}

	static inline long getBalance(AVLLinker *ptr)
	{
		return (ptr) ? (getHeight(ptr->leftChild) - getHeight(ptr->rightChild)) : 0;
	}
};

/**
 * Struct: AVLNode
 *
 * Summary:
 * This is the standard element of the AVL-tree and should be embedded in other
 * structs to used them in avl-trees.
 *
 * Version: 2.01
 * Author: Shukant Pal
 */
struct AVLNode
{
	AVLNode *leftChild;
	AVLNode *rightChild;
	unsigned long depth;
	unsigned long sortValue;

	static inline long getHeight(AVLNode *ptr)
	{
		return (ptr) ? (ptr->depth) : -1;
	}

	static inline long getBalance(AVLNode *ptr)
	{
		return (ptr) ? (getHeight(ptr->leftChild) - getHeight(ptr->rightChild)) : 0;
	}
};

struct AVLTree
{
	AVLNode *treeRoot;
	SIZE nodeCount;
};

enum
{
	NO_RETURN,
	NODE_INSERTED,
	NODE_FOUND
};

decl_c unsigned long AVLInsert(AVLNode *Node, AVLTree *Root);
decl_c AVLNode *AVLDelete(SIZE Indicator, AVLTree *Tree);
decl_c AVLNode *AVLSearch(SIZE Indicator, AVLTree *Tree);
decl_c AVLNode *MinValueNode(AVLNode *Node);
decl_c AVLNode *MaxValueNode(AVLNode *Node);
decl_c AVLNode *AVLFindGTE(unsigned long nodeValue, AVLTree *tree);

#endif/* Util/AVLTree.hpp */
