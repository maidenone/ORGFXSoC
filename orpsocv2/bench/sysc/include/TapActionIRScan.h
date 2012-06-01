// ----------------------------------------------------------------------------

// TAP IR-Scan action: definition

// Copyright (C) 2009  Embecosm Limited <info@embecosm.com>

// Contributor Jeremy Bennett <jeremy.bennett@embecosm.com>

// This file is part of the Embecosm cycle accurate SystemC JTAG library.

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

// The C/C++ parts of this program are commented throughout in a fashion
// suitable for processing with Doxygen.

// ----------------------------------------------------------------------------

// $Id$

#ifndef TAP_ACTION_IR_SCAN__H
#define TAP_ACTION_IR_SCAN__H

#include <stdint.h>

#include "TapAction.h"
#include "TapStateMachine.h"

//! Class to represent a TAP IR-Scan action.

//! This class assumes that JTAG instruction registers are relatively
//! small. IEEE 1149.1 mandates at least 2 bits, although implementations
//! often have a wider range of instructions. The OpenRISC 1000 debug unit for
//! example uses 5 bits. 32-bits seems more than enough for the largest
//! applications.

class TapActionIRScan:public TapAction {
public:

	// Constructor
	TapActionIRScan(sc_core::sc_event * _doneEvent,
			uint32_t _iRegIn, int _iRegSize);

	// Get the shifted out value
	uint32_t getIRegOut();

protected:

	// Process the action for IR-Scan
	 bool process(TapStateMachine * tapStateMachine,
		      bool & tdi, bool tdo, bool & tms);

private:

	//! The value being shifted in
	 uint32_t iRegIn;

	//! The number of bits to shift
	int iRegSize;

	//! The value shifted out
	uint32_t iRegOut;

	//! The number of bits shifted so far
	int bitsShifted;

	//! Where we are in the IR-scan process
	enum {
		SHIFT_IR_PREPARING,
		SHIFT_IR_SHIFTING,
		SHIFT_IR_UPDATING
	} iRScanState;

};				// TapActionIRScan

#endif // TAP_ACTION_IR_SCAN__H
