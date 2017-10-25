/* Copyright (C) 2017 - Shukant Pal */

#include <Circuit.h>
#include <Util/AVLTree.h>

/**
 * AVLRotateRight() - 
 *
 * Summary:
 * This function is used for rotating the subtree (towards the right).
 *
 * Args:
 * node - Subtree to rotate
 *
 * @Version 1.1
 * @Since Circuit 2.01 
 */
static
AVL_LINKER *AVLRotateRight(AVL_LINKER *node){
	AVL_LINKER *X = node->Left;
	AVL_LINKER *Y = X->Right;

	X->Right  = node;
	node->Left = Y;

	node->Height = Max(AVLHeight(node->Left), AVLHeight(node->Right)) + 1;
	X -> Height = Max(AVLHeight(X -> Left), AVLHeight(X -> Right)) + 1;

	return (X);
}

/**
 * AVLRotateLeft() - 
 *
 * Summary:
 * This function is used for rotating the subtree (towards the left).
 *
 * Args:
 * newNode - Subtree to rotate
 *
 * @Version 1.1
 * @Since Circuit 2.01
 */
static
AVL_LINKER *AVLRotateLeft(AVL_LINKER *newNode){
	AVL_LINKER *Y = AVLRight(newNode);
	AVL_LINKER *X = AVLLeft(Y);

	Y->Left = newNode;
	newNode->Right = X;

	newNode->Height = Max(AVLHeight(newNode -> Left), AVLHeight(newNode -> Right)) + 1;
	Y->Height = Max(AVLHeight(Y -> Left), AVLHeight(Y -> Right)) + 1;

	return (Y);
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
static
AVLNODE *InsertNodeInBranch(AVLNODE *newNode, ULONG *insertionStatus, AVLNODE *rootNode){
	if(rootNode == NULL){
		*insertionStatus = NODE_INSERTED;
		return (newNode);
	}

	if(newNode->Indicator < rootNode->Indicator) {
		rootNode->Left = InsertNodeInBranch(newNode, insertionStatus, rootNode->Left);
	} else if(newNode->Indicator > rootNode->Indicator) {
		rootNode->Right = InsertNodeInBranch(newNode, insertionStatus, rootNode->Right);
	} else {
		*insertionStatus = NODE_FOUND;
		return (rootNode); // Indicate that matching root is found.
	}

	AVLNODE *leftChild = rootNode->Left;
	AVLNODE *rightChild = rootNode->Right;
	ULONG nodeIndicator = newNode->Indicator;

	rootNode->Height = Max(AVLHeight(leftChild), AVLHeight(rightChild)) + 1;
	LONG nodeBalance = AVLBalance(rootNode);

	if(nodeBalance >  1) {
		if(nodeIndicator < leftChild->Indicator){
			return (AVLNODE*) (AVLRotateRight((AVL_LINKER*) rootNode));
		} else if(nodeIndicator > leftChild->Indicator){
			rootNode->Left = (AVLNODE*) AVLRotateLeft((AVL_LINKER*) leftChild);
			return (AVLNODE*) (AVLRotateRight((AVL_LINKER*) rootNode));
		}
	} else if(nodeBalance < -1) {
		if(nodeIndicator > rightChild->Indicator){
			return (AVLNODE*) AVLRotateLeft((AVL_LINKER*) rootNode);
		} else if(nodeIndicator < rightChild->Indicator){
			rootNode->Right = (AVLNODE*) AVLRotateRight((AVL_LINKER*) rightChild);
			return (AVLNODE*) AVLRotateLeft((AVL_LINKER*) rootNode);
		}
	}

	return (rootNode); // newNode could not be inserted
}

AVLNODE *AVLSearch_(SIZE_T Indicator, AVLNODE *newNode){
	if(newNode == NULL)
		return (NULL);

	if(Indicator < AVLIndicator(newNode))
		return (AVLSearch_(Indicator, newNode -> Left));
	else if(Indicator > AVLIndicator(newNode))
		return (AVLSearch_(Indicator, newNode -> Right));
	else
		return (newNode);
}

AVLNODE *MinValueNode(AVLNODE *newNode){
	AVLNODE *Current = newNode;
	if(Current == NULL) return NULL;

	while(Current -> Left != NULL)
		Current = Current -> Left;

	return (Current);
}

AVLNODE *MaxValueNode(AVLNODE *newNode){
	AVLNODE *Current = newNode;
	if(Current == NULL) return (NULL);

	while(Current-> Right != NULL)
		Current = Current -> Right;

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
static
AVLNODE *DeleteNodeInBranch(SIZE_T Indicator, AVLNODE **deletedNode, AVLNODE *rootNode){
	if(rootNode == NULL)
		return (rootNode);

	if(Indicator < rootNode->Indicator){
		rootNode->Left= DeleteNodeInBranch(Indicator, deletedNode, rootNode->Left);
	} else if(Indicator > rootNode->Indicator){
		rootNode->Right = DeleteNodeInBranch(Indicator, deletedNode, rootNode->Right);
	} else {
		*deletedNode = rootNode;
		AVLNODE *childNode; // This node is to be deleted (freed).
		if((rootNode->Left == NULL) || (rootNode->Right == NULL)) {
			childNode = (rootNode->Left) ? rootNode->Left : rootNode->Right;

			if(childNode == NULL) {
				childNode = rootNode;
				rootNode = NULL;
			} else {
				rootNode = childNode;
				childNode = rootNode;
			}
		} else {
			AVLNODE *rightHalver = MinValueNode(rootNode->Right);
			AVLNODE *newRoot = DeleteNodeInBranch(rightHalver->Indicator, deletedNode, rootNode->Right);
			rightHalver->Right = newRoot;
			rightHalver->Left = rootNode->Left;
			rootNode = rightHalver;
		}
	}

	if(rootNode == NULL)
		return (NULL);

	rootNode->Height = Max(AVLHeight(rootNode->Left), AVLHeight(rootNode->Right)) + 1;
	SSIZE_T nodeBalance = AVLBalance(rootNode);

	if(nodeBalance > 1) {
		if(AVLBalance(rootNode->Left) >= 0) {
			return (AVLNODE*) AVLRotateRight((AVL_LINKER*) rootNode);
		} else if(AVLBalance(rootNode->Left) < 0){
			rootNode->Left = (AVLNODE*) AVLRotateLeft((AVL_LINKER*) rootNode->Left);
			return (AVLNODE*) AVLRotateRight((AVL_LINKER*) rootNode);
		}
	} else if(nodeBalance < -1) {
		if(AVLBalance(rootNode->Right) <= 0) {
			return (AVLNODE*) AVLRotateLeft((AVL_LINKER*) rootNode);
		} else if(AVLBalance(rootNode->Right) > 0) {
			rootNode->Right = (AVLNODE*) AVLRotateRight((AVL_LINKER*) rootNode->Right);
			return (AVLNODE*) AVLRotateLeft((AVL_LINKER*) rootNode);
		}
	}

	return (rootNode);
}

AVLNODE *AVLFindGTE(ULONG nodeValue, AVLTREE *tree){
	AVLNODE *curNode = tree->Root;
	AVLNODE *closestNode = closestNode;

	ULONG curValue;
	ULONG curDiff = 0;
	ULONG lastDiff = 0xFFFFFFFF;

	while(curNode != NULL){
		curValue = curNode->Indicator;
		curDiff = curValue - nodeValue;
		if(curDiff < lastDiff) {
			closestNode = curNode;
			if(curDiff == 0)
				break;
		}

		if(nodeValue < curValue)
			curNode = curNode->Right;
		else
			curNode = curNode->Left;
		/* Equal-to case already resolved */
	}

	return (closestNode);
}

ULONG AVLInsert(AVLNODE *newNode, AVL_TREE *Tree){
	ULONG insertionStatus;

	Tree ->Root = InsertNodeInBranch(newNode, &insertionStatus, Tree -> Root);
	if(insertionStatus == NODE_INSERTED){
		++(Tree->Size);
	}

	return (insertionStatus);
}

AVLNODE *AVLDelete(SIZE_T Indicator, AVL_TREE *nodeTree){
	AVLNODE *deletedNode = NULL;

	nodeTree->Root = DeleteNodeInBranch(Indicator, &deletedNode, nodeTree->Root);
	if(deletedNode != NULL){
		--(nodeTree->Size);
	}

	return (deletedNode);
}

AVLNODE *AVLSearch(SIZE_T Indicator, AVL_TREE *Tree){
	return AVLSearch_(Indicator, Tree -> Root);
}
