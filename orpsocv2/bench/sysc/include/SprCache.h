// ----------------------------------------------------------------------------

// Debug Unit SPR cache: definition

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

// $Id: SprCache.h 331 2009-03-12 17:01:48Z jeremy $

#ifndef SPR_CACHE__H
#define SPR_CACHE__H

#include <stdint.h>

//-----------------------------------------------------------------------------
//! Module for cacheing SPR accesses by the debug unit

//! SPR reads and writes through the Debug Unit via JTAG are time
//! consuming - of the order of 1000 CPU clock cycles. However when the
//! processor is stalled the values cannot change, other than through the
//! debug unit, so it makes sense to cache values.

//! @note It is not strictly true that SPRs do not change. If the NPC is
//!       written, it flushes the pipeline, and subsequent reads will return
//!       zero until the processor is unstalled and the pipeline has
//!       refilled. However for our purposes, it is convenient to return the
//!       value written into the NPC in such circumstances.
//!
//! The cache is represented as a closed hash table, which is generally
//! allowed to be no more than 70% full (however NPC is always
//! cacheable). The hash function is a simple modulo function, stepping
//! forward to the first free slot. This works because there is no function to
//! delete an entry - just to clear the whole table, so holes cannot appear.
//-----------------------------------------------------------------------------
class SprCache {
public:

	// Constructor and destructor
	SprCache(int _tableSize = 257);
	~SprCache();

	// Functions
	void clear();
	void write(uint16_t sprNum, uint32_t value, bool force);
	bool read(uint16_t sprNum, uint32_t & value);

private:

	//! The size of the hash table
	int tableSize;

	//! Maximum amount of cache left to use, before cacheing is rejected.
	int maxToUse;

	// The cache, keyed by sprNum. Done as two parallel vectors,
	// allowing unambiguous clearing by use of memset for efficiency.
	bool *sprIsValid;
	uint16_t *sprKeyNum;
	uint32_t *sprValue;

};				// SprCache ()

#endif // SPR_CACHE__H
