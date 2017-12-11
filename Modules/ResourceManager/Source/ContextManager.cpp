/**
 * File: ContextManager.cpp
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
#include <Resource/ContextManager.hpp>

using namespace Resource;

ObjectInfo *ContextManager::tMemorySection;

ContextManager::ContextManager(
		const char *mgrName,
		const int typeIdt
)
: name(mgrName), typeId(typeIdt)
{
	this->code = this->data = this->bss = this->mainStack =
			this->referCount = 0;
	this->recentCache = NULL;
}

ContextManager::~ContextManager()
{

}
