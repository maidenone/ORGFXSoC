// ----------------------------------------------------------------------------

// Debug Unit memory cache: implementation

// Copyright (C) 2008  Embecosm Limited <info@embecosm.com>

// Contributor Jeremy Bennett <jeremy.bennett@embecosm.com>

// This file is part of the GDB interface to the cycle accurate model of the
// OpenRISC 1000 based system-on-chip, ORPSoC, built using Verilator.

// This program is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or (at your
// option) any later version.

// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
// License for more details.

// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// ----------------------------------------------------------------------------

// $Id: MemCache.cpp 326 2009-03-07 16:47:31Z jeremy $

#include <cstring>

#include "MemCache.h"

//! Constructor

//! Allocate a closed hash table of the specified size and clear it.

//! @param[in] _tableSize  The desire hash table size. A prime number is
//!                         recommended.

MemCache::MemCache(int _tableSize):tableSize(_tableSize)
{
	tabIsValid = new bool[tableSize];
	tabKeyAddr = new uint32_t[tableSize];
	tabValue = new uint32_t[tableSize];

	clear();

}				// MemCache ()

//! Destructor

//! Free the hash table arrays

MemCache::~MemCache()
{
	delete[]tabIsValid;
	delete[]tabKeyAddr;
	delete[]tabValue;

}				// ~MemCache ()

//! Empty the hash table

//! Only need to worry about the validity field
void
 MemCache::clear()
{
	memset(tabIsValid, false, sizeof(tabIsValid));

}				// clear ()

//! Write a new value into the hash table

//! Will trash anything already there.

//! @param[in] addr   The address being written to
//! @param[in] value  The value to write
void MemCache::write(uint32_t addr, uint32_t value)
{
	int keyAddr = addr % tableSize;

	tabIsValid[keyAddr] = true;
	tabKeyAddr[keyAddr] = addr;
	tabValue[keyAddr] = value;

}				// write ()

//! Try to read a value from the hash table

//! The entry must be valid and the address must match

//! @param[in]  addr   The address being read from
//! @param[out] value  The value read, if there was one there

//! @return  True if the value was found in the hash table

bool MemCache::read(uint32_t addr, uint32_t & value)
{
	int keyAddr = addr % tableSize;

	if (tabIsValid[keyAddr] & (tabKeyAddr[keyAddr] == addr)) {
		value = tabValue[keyAddr];
		return true;
	} else {
		return false;
	}
}				// read ()
