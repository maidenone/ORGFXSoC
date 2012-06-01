// ----------------------------------------------------------------------------

// TAP DR-Scan action: definition

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

#ifndef TAP_ACTION_DR_SCAN__H
#define TAP_ACTION_DR_SCAN__H

#include <stdint.h>

#include "TapAction.h"
#include "TapStateMachine.h"

//! Class to represent a TAP DR-Scan action.

//! JTAG data registers can be huge and are represented generically as arrays
//! of uint64_t. However for efficiency versions of methods are provided based
//! on a single uint64_t.

//! The SystemC classes for large unsigned ints are fabulously inefficient in
//! the reference implementation, so are not used here.

class TapActionDRScan:public TapAction {
public:

	// Constructors and destructor
	TapActionDRScan(sc_core::sc_event * _doneEvent,
			uint64_t _dRegInArray[], int _dRegSize);
	 TapActionDRScan(sc_core::sc_event * _doneEvent,
			 uint64_t _dRegIn, int _dRegSize);
	 TapActionDRScan(sc_core::sc_event * _doneEvent,
			 uint64_t _dRegInArray[],
			 int _dRegSize,
			 int _goToPauseState, int _bitsBeforePauseState);
	 TapActionDRScan(sc_core::sc_event * _doneEvent,
			 uint64_t _dRegIn,
			 int _dRegSize,
			 int _goToPauseState, int _bitsBeforePauseState);
	~TapActionDRScan();

	// Get the shifted out value
	void getDRegOut(uint64_t dRegArray[]);
	uint64_t getDRegOut();

protected:

	// Process the action for IR-Scan
	 bool process(TapStateMachine * tapStateMachine,
		      bool & tdi, bool tdo, bool & tms);

private:

	//! Number of bits in the data register
	int dRegBitSize;

	//! Number of uint64_t words in the data register
	int dRegWordSize;

	//! Mask for top word in multi-word register
	uint64_t topMask;

	//! The value being shifted in
	uint64_t *dRegInArray;

	//! The value being shifted in (small version optimization)
	uint64_t dRegIn;

	//! The value shifted out
	uint64_t *dRegOutArray;

	//! The value being shifted out (small version optimization)
	uint64_t dRegOut;

	//! Should we go to PAUSE state and poll tdo during operation?
	int goToPauseState;

	//! Number of bits to shift before going to PAUSE state and polling tdo
	int bitsBeforePause;

	int pauseStateCount;

	//! Bits shifted so far
	int bitsShifted;

	//! Where we are in the Shift-DR process
	enum {
		SHIFT_DR_PREPARING,
		SHIFT_DR_SHIFTING,
		SHIFT_DR_SHIFTING_BEFORE_PAUSE,
		SHIFT_DR_SHIFTING_PAUSE,
		SHIFT_DR_EXIT2,
		SHIFT_DR_SHIFTING_AFTER_PAUSE,
		SHIFT_DR_UPDATING
	} dRScanState;

	// Utilities to shift the bottom bit out and top bit in
	bool shiftDRegOut();
	void shiftDRegIn(bool bitIn);

};				// TapActionDRScan

#endif // TAP_ACTION_DR_SCAN__H
