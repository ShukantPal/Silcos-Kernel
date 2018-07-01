///
/// @file NodeSorter.hpp
///
/// Provides a container for holding EventNode objects in a sorted
/// data structure - particularly, the red-black tree. This container
/// fulfills the requirement of having cached minimum and maximum nodes
/// so that the EventQueue object can easily push and pull time ranges.
/// -------------------------------------------------------------------
/// This program is free software: you can redistribute it and/or modify
/// it under the terms of the GNU General Public License as published by
/// the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program.  If not, see <http://www.gnu.org/licenses/>
///
/// Copyright (C) 2017 - Shukant Pal
///

#ifndef EXECMGR_TIMER_NODESORTER_HPP__
#define EXECMGR_TIMER_NODESORTER_HPP__

#include "EventNode.hpp"

namespace Executable
{
namespace Timer
{

class NodeSorter
{
public:
	NodeSorter();
	void del(EventNode *oldNode);
	EventNode *findFor(EventTrigger *constraint);
	void put(EventNode *newNode);
private:
	EventNode *treeRoot, *nil;
	EventNode *mostRecent, *mostLate;
	unsigned long nodeCount;

	EventNode *getInitialLeaf(EventNode *newLeaf,
			EventNode *treeRoot);
	EventNode *leanLeft(EventNode *localRoot);
	EventNode *leanRight(EventNode *localRoot);
	void repairTree(EventNode *newLeaf);

	void replaceRoot(EventNode *old, EventNode *with) {
		with->parent = old->parent;

		EventNode *nParent = old->parent;
		if(nParent != nil) {
			if(old->isLeftChild())
				nParent->leftChild = with;
			else
				nParent->rightChild = with;
		}
	}
};

} // namespace Timer
} // namespace Executable

#endif/* Executable/Timer/NodeSorter.hpp */
