/**
 * File: MemoryImage.cpp
 *
 * Summary:
 * 
 * Functions:
 *
 * Origin:
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
#include <Process/MemoryImage.hpp>

using namespace Resource;
using namespace Process;

RegionInsertionResult MemoryImage::insertRegion(
		unsigned long initialAddress,
		unsigned long pageCount,
		unsigned short ctlFlags,
		PAGE_ATTRIBUTES pageFlags
){
	MemorySection *region = new(tMemorySection) MemorySection
				(initialAddress, pageCount,
				(psmmManager << 16) | ctlFlags, pageFlags);


}

/**
 * Function: MemoryImage::findRegion
 *
 * Summary:
 * Finds the region which contains the given address and uses the tree
 * or linked-list based on the treeUsed (bool) flag.
 */
MemorySection* MemoryImage::findRegion(
		unsigned long address
){
	if(treeUsed){
		MemorySection *closestMatch = // lower becauz starting is <
				(MemorySection*) sectionTree->getLowerBoundFor
								(address);

		if(closestMatch && closestMatch->finalAddress > address)
			return (closestMatch);
		else
			return (NULL);
	} else {
		MemorySection *tSection = (MemorySection*) regionChain.Head;

		while(tSection != NULL){
			if(tSection->initialAddress <= address &&
					tSection->finalAddress > address){
				return (tSection);
			}

			tSection = tSection->nextSection;
		}

		return (NULL);
	}
}
