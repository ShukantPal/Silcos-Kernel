///
/// @file BinaryHeap.hpp
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

#ifndef BINARYHEAP_HPP_
#define BINARYHEAP_HPP_

#include <Heap.hpp>
#include "Memory.h"

///
///
///
/// Requirements on BH_Node:
///
/// BH_Node is the nodal member of the binary heap. The BinaryHeap class uses
/// its methods to access its value, initialize it, and de-initialize it. This
/// is required so that construction/destruction overhead is eliminated. The
/// required methods are (@code getHeapValue(), setHeapValue(), init(),
/// deinit()) along with a default constructor.
///
///
template<class BH_Node>
class BinaryHeap final
{
public:
	///
	/// Allocates the nodal table for the binary heap with the default
	/// buffer size. No initialization of nodes is done while constructing
	/// the heap.
	///
	BinaryHeap()
	{
		bufferSize = 8;
		nodeCount = usefulNodes = 0;
		nodalTable = kmalloc(8 * sizeof(BH_Node));
	}

	///
	/// Allocates the nodal table for the binary heap with the given
	/// initial buffer size. No initialization of nodes is done while
	/// constructing the heap.
	///
	/// @param initialNodes - the initial buffer size for the heap. This
	/// 		should be about the number of nodes the heap will
	/// 		contain. Passing this argument saves insertion
	/// 		overhead when the buffer actually is expanded.
	///
	BinaryHeap(int initialNodes)
	{
		bufferSize = initialNodes * sizeof(BH_Node);
		nodeCount = usefulNodes = 0;
		nodalTable = kmalloc(initialNodes * sizeof(BH_Node));
	}

	BH_Node *newNode(int hvl)
	{
		if(usefulNodes == nodeCount)
			new((void*)(nodalTable + usefulNodes - 1)) BH_Node();

		nodalTable[nodeCount++].setHeapValue(hvl);


	}

	void deleteNode()
	{

	}

private:
	BH_Node *nodalTable;//!< The array containing the binary heap nodes
	 	 	    //! allocated from the memory-heap.
	int nodeCount;//!< The no. of nodes actually in the tree currently.
	int usefulNodes;//!< The no. of nodes initialized and are directly
	 	 	//! usable from the array.
	int bufferSize;//!< Size of the nodalTable allocated from memory-heap.
};

#endif/* Utils/BinaryHeap.hpp */
