// ----------------------------------------------------------------------------

// Debug Unit memory cache: definition

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

// $Id: MemCache.h 326 2009-03-07 16:47:31Z jeremy $

#ifndef MEM_CACHE__H
#define MEM_CACHE__H

#include <stdint.h>

//! Module for cacheing memory accesses by the debug unit

//! Memory reads and writes through the Debug Unit via JTAG are time
//! consuming - of the order of 1000 CPU clock cycles. However when the
//! processor is stalled the values cannot change, other than through the
//! debug unit, so it makes sense to cache values.

//! Cacheing the entire memory is too much (it does need to be cleared when
//! the processor is unstalled. This class provides a cacheing function using
//! a closed hash table.

//! In the event of a clash on write, the old value is replaced by the new
//! value.

class MemCache {
public:

	// Constructor and destructor
	MemCache(int _tableSize = 1009);
	~MemCache();

	// Functions
	void clear();
	void write(uint32_t addr, uint32_t value);
	bool read(uint32_t addr, uint32_t & value);

private:

	//! The size of the hash table. A prime number is a good choice.
	int tableSize;

	// The hash table, keyed by address. Done as three parallel vectors,
	// allowing unambiguous clearing by use of memset for efficiency.
	bool *tabIsValid;
	uint32_t *tabKeyAddr;
	uint32_t *tabValue;

};				// MemCache ()

#endif // MEM_CACHE__H
