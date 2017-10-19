/**
 * Copyright (C) 2017 - Shukant Pal
 *
 * The microkernel generally uses AVL tree for services such as
 * scheduling and data management. The header provides the interface
 * for using the tree data structure.
 */
#ifndef UTIL_AVL_TREE_H
#define UTIL_AVL_TREE_H

#include <Types.h>

#define AVLLeft(N) (N -> Left)
#define AVLRight(N) (N -> Right)
#define AVLHeight(N) (LONG) (N ? (N -> Height) : -1)

#define BALANCE_HEIGHT(N) (LONG) (N ? (N->Height) : -1)
#define AVLBalance(N) (LONG) (N ? (BALANCE_HEIGHT(N -> Left) - BALANCE_HEIGHT(N -> Right)) : 0)
#define AVLIndicator(N) (N -> Indicator)
#define AVLReferer(N) (N -> Referer)

typedef
struct _AVL_LINKER {
	struct _AVL_LINKER *Left;
	struct _AVL_LINKER *Right;
	ULONG Height;
} AVL_LINKER;

/**
 * AVLNODE - 
 *
 * Summary:
 * This type represents the AVL node, which is used for keeping others in
 * a AVL tree.
 *
 * Fields:
 * Left - Left linker
 * Right - Right linker
 * Height - Height of the subtree
 * Indicator - Value of the node
 *
 * @Version 1.1
 * @Since Circuit 2.01
 */
typedef
struct _AVLNODE {
	struct _AVLNODE *Left;
	struct _AVLNODE *Right;
	ULONG Height;
	ULONG Indicator;
} AVLNODE;

#define AVLRoot(T) (T -> Root)

#define NODE_NO_REFER (1 << 0)
#define NNR 1

typedef
struct _AVL_TREE
{
	VOID *Metadata;
	AVLNODE *Root;
	SIZE Size;
	ULONG NodeConfig;
	VOID (*AllocateNode) (ULONG Indicator); /* Used in inter-module comm. */
	VOID (*FreeNode) (AVLNODE *nodePointer); /* Used inter-module comm. */
} AVL_TREE;

typedef AVL_TREE AVLTREE;

#define AVL_Tree { .Metadata = NULL, .NodeConfig = NULL, .Size = 0, .Root = NULL }

#define NO_RETURN 0 /* Error */
#define NODE_INSERTED 1 /* Node was inserted */
#define NODE_FOUND 2 /* N*/

/**
 * AVLInsert() - 
 *
 * Summary:
 * This function inserts the AVL node into the AVL tree given.
 *
 * Args:
 * Node - to insert
 * Tree - used AVL tree
 *
 * Returns: Insertion status
 *
 * @Version 1.1
 * @Since Circuit 2.01
 */
ULONG AVLInsert(
	AVLNODE *Node,
	AVL_TREE *Root
);

/**
 * AVLDelete() - 
 *
 * Summary:
 * This function removes the node with the given value from the AVL tree given.
 *
 * Args:
 * Indicator - Value to return
 * Tree - used AVL tree
 *
 * Returns: Removed AVLNODE
 *
 * @Version 1.1
 * @Since Circuit 2.01
 */
AVLNODE *AVLDelete(
	SIZE_T Indicator,
	AVL_TREE *Tree
);

AVLNODE *AVLSearch(
	SIZE_T Indicator, 
	AVL_TREE *Tree
);

AVLNODE *MinValueNode(
	AVLNODE *Node
);

#define LeastNode MinValueNode

AVLNODE *MaxValueNode(
	AVLNODE *Node
);

AVLNODE *AVLFindGTE(
	ULONG nodeValue,
	AVLTREE *tree
);

#endif /* Util/AVLTree.h */
