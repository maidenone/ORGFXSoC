// ----------------------------------------------------------------------------

// Debug Unit SPR cache: implementation

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

// $Id: SprCache.cpp 331 2009-03-12 17:01:48Z jeremy $

#include <iostream>
#include <cstring>

#include "SprCache.h"

//-----------------------------------------------------------------------------
//! Constructor

//! Allocate tables and clear the cache

//! @param[in] _tableSize  The desire hash table size. A prime number is
//!                         recommended.
//-----------------------------------------------------------------------------
SprCache::SprCache(int _tableSize):
tableSize(_tableSize)
{
	sprIsValid = new bool[tableSize];
	sprKeyNum = new uint16_t[tableSize];
	sprValue = new uint32_t[tableSize];

	clear();

}				// SprCache ()

//-----------------------------------------------------------------------------
//! Destructor

//! Free up the tables
//-----------------------------------------------------------------------------
SprCache::~SprCache()
{
	delete[]sprValue;
	delete[]sprKeyNum;
	delete[]sprIsValid;

}				// ~SprCache ()

//! Empty the hash table

//! Only need to worry about the validity field
void
 SprCache::clear()
{
	memset(sprIsValid, false, tableSize);

	// No more than 70% full
	maxToUse = tableSize * 7 / 10;

}				// clear ()

//-----------------------------------------------------------------------------
//! Write a new value into the cache

//! If the hash table is full silently does nothing, unless the force
//! parameter is set to TRUE. Under this circumstance the value WILL be
//! written into the hash table. This is safe, because the table is never more
//! than 70% full, and force is used only for NPC.

//! @param[in] spr    The SPR being written to
//! @param[in] value  The value to write
//! @param[in] force  If TRUE the value will be written to the hash table,
//!                   even if it is too full.
//-----------------------------------------------------------------------------
void SprCache::write(uint16_t sprNum, uint32_t value, bool force)
{
	if (maxToUse <= 0) {
		return;		// Table is full
	}

	int hv = sprNum % tableSize;

	// We can use the slot if either it is empty, or it is full and the key
	// number matches.
	while (sprIsValid[hv] && (sprKeyNum[hv] != sprNum)) {
		hv = (hv + 1) % tableSize;
	}

	sprIsValid[hv] = true;
	sprKeyNum[hv] = sprNum;
	sprValue[hv] = value;
	maxToUse--;

}				// write ()

//-----------------------------------------------------------------------------
//! Try to read a value from the cache

//! The entry must be valid.

//! @param[in]  sprNum  The SPR being read from
//! @param[out] value   The value read. Will be written, even if the value is
//!                     not valid.

//! @return  True if the value was found in the hash table
//-----------------------------------------------------------------------------
bool SprCache::read(uint16_t sprNum, uint32_t & value)
{
	int hv = sprNum % tableSize;

	// Look for either an empty slot (we are not there) or a matching key (we
	// are there)
	while (sprIsValid[hv] && (sprKeyNum[hv] != sprNum)) {
		hv = (hv + 1) % tableSize;
	}

	value = sprValue[hv];
	return sprIsValid[hv];

}				// read ()
