// ----------------------------------------------------------------------------

// TAP DR-Scan action: implementation

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

#include <iostream>
#include <stdio.h>
#include "TapActionDRScan.h"
#include "TapStateMachine.h"

//! Constructor for "large" DR registers

//! Sets up the superclass with the SystemC completion event and initializes
//! our state as appropriate.

//! This constructor represents large registers as an array of uint64_t, with
//! least significant bits in the lowest numbered element, and any odd bits in
//! the highest numbered element.

//! However if we are presented with an array that represents a "small"
//! (i.e. up to 64-bit) register, we will store it efficiently.

//! @param[in] doneEvent     SystemC event to be signalled when this action is
//!                          complete.
//! @param[in] _dRegInArray  The register to shift in.
//! @param[in] _dRegSize     Size in bits of the register to shift in.

TapActionDRScan::TapActionDRScan(sc_core::sc_event * _doneEvent, uint64_t * _dRegInArray, int _dRegSize):
TapAction(_doneEvent),
dRegBitSize(_dRegSize),
dRegWordSize((_dRegSize + 63) / 64),
goToPauseState(0),
bitsBeforePause(0), bitsShifted(0), dRScanState(SHIFT_DR_PREPARING)
{
	// Small registers are represented simply. Large registers are copied to a
	// local instance (since we destroy dRegIn when shifting it)

	if (1 == dRegWordSize) {
		dRegIn = _dRegInArray[0];
		dRegOut = 0;
	} else {
		dRegInArray = new uint64_t[dRegWordSize];
		dRegOutArray = new uint64_t[dRegWordSize];

		// Copy in the in array and zero the out array
		for (int i = 0; i < dRegWordSize; i++) {
			dRegInArray[i] = _dRegInArray[i];
			dRegOutArray[i] = 0;
		}

		// Create a mask for the top word
		int maskBits = ((dRegBitSize - 1) % 64) + 1;
		topMask = (1ULL << maskBits) - 1ULL;
	}
}				// TapActionDRScan ()

//! Constructor for small DR registers

//! Sets up the superclass with the SystemC completion event and initializes
//! our state as appropriate.

//! This constructor represents small registers in a single uint64_t

//! @param[in] doneEvent  SystemC event to be signalled when this action is
//!                       complete.
//! @param[in] _dRegIn    The register to shift in.
//! @param[in] _dRegSize  Size in bits of the register to shift in. Must be no
//!                       greater than 64, or we give a rude message and set
//!                       the value to 64 anyway.

TapActionDRScan::TapActionDRScan(sc_core::sc_event * _doneEvent,
				 uint64_t _dRegIn,
				 int _dRegSize):TapAction(_doneEvent),
dRegBitSize(_dRegSize),
dRegWordSize(1),
goToPauseState(0),
bitsBeforePause(0), bitsShifted(0), dRScanState(SHIFT_DR_PREPARING)
{
	// Print a rude message if we are not small
	if (dRegBitSize > 64) {
		std::cerr << "Simple DR size reduced to 64 bits" << std::endl;
		dRegBitSize = 64;
	}
	// Simple representation
	dRegIn = _dRegIn;
	dRegOut = 0;

}				// TapActionDRScan ()

//! Constructor for "large" DR registers using special PAUSE state

//! Sets up the superclass with the SystemC completion event and initializes
//! our state as appropriate.

//! This constructor represents large registers as an array of uint64_t, with
//! least significant bits in the lowest numbered element, and any odd bits in
//! the highest numbered element.

//! However if we are presented with an array that represents a "small"
//! (i.e. up to 64-bit) register, we will store it efficiently.

//! @param[in] doneEvent     SystemC event to be signalled when this action is
//!                          complete.
//! @param[in] _dRegInArray  The register to shift in.
//! @param[in] _dRegSize     Size in bits of the register to shift in.
//! @param[in] _goToPauseState     Switch determining if we go to PAUSE state after _bitsBeforePauseState and poll for TDO=0
//! @param[in] _bitsBeforePauseState     Number of bits to shift in before going to shift pause state and polling TDO, indicating transaction has completed

TapActionDRScan::TapActionDRScan(sc_core::sc_event * _doneEvent,
				 uint64_t * _dRegInArray,
				 int _dRegSize,
				 int _goToPauseState,
				 int
				 _bitsBeforePauseState):TapAction(_doneEvent),
dRegBitSize(_dRegSize), dRegWordSize((_dRegSize + 63) / 64),
goToPauseState(_goToPauseState), bitsBeforePause(_bitsBeforePauseState),
pauseStateCount(0), bitsShifted(0), dRScanState(SHIFT_DR_PREPARING)
{
	// Small registers are represented simply. Large registers are copied to a
	// local instance (since we destroy dRegIn when shifting it)

	if (1 == dRegWordSize) {
		dRegIn = _dRegInArray[0];
		dRegOut = 0;
	} else {
		dRegInArray = new uint64_t[dRegWordSize];
		dRegOutArray = new uint64_t[dRegWordSize];

		// Copy in the in array and zero the out array
		for (int i = 0; i < dRegWordSize; i++) {
			dRegInArray[i] = _dRegInArray[i];
			dRegOutArray[i] = 0;
		}

		// Create a mask for the top word
		int maskBits = ((dRegBitSize - 1) % 64) + 1;
		topMask = (1ULL << maskBits) - 1ULL;
	}
}				// TapActionDRScan ()

//! Constructor for small DR registers using special PAUSE state

//! Sets up the superclass with the SystemC completion event and initializes
//! our state as appropriate.

//! This constructor represents small registers in a single uint64_t

//! @param[in] doneEvent  SystemC event to be signalled when this action is
//!                       complete.
//! @param[in] _dRegIn    The register to shift in.
//! @param[in] _dRegSize  Size in bits of the register to shift in. Must be no
//!                       greater than 64, or we give a rude message and set
//!                       the value to 64 anyway.
//! @param[in] _goToPauseState     Switch determining if we go to PAUSE state after _bitsBeforePauseState and poll for TDO=0
//! @param[in] _bitsBeforePauseState     Number of bits to shift in before going to shift pause state and polling TDO, indicating transaction has completed

TapActionDRScan::TapActionDRScan(sc_core::sc_event * _doneEvent,
				 uint64_t _dRegIn,
				 int _dRegSize,
				 int _goToPauseState,
				 int
				 _bitsBeforePauseState):TapAction(_doneEvent),
dRegBitSize(_dRegSize), dRegWordSize(1), goToPauseState(_goToPauseState),
bitsBeforePause(_bitsBeforePauseState), pauseStateCount(0), bitsShifted(0),
dRScanState(SHIFT_DR_PREPARING)
{
	// Print a rude message if we are not small
	if (dRegBitSize > 64) {
		std::cerr << "Simple DR size reduced to 64 bits" << std::endl;
		dRegBitSize = 64;
	}
	// Simple representation
	dRegIn = _dRegIn;
	dRegOut = 0;

}				// TapActionDRScan ()

//! Destructor.

//! If we allocated them, free the large registers

TapActionDRScan::~TapActionDRScan()
{
	if (dRegWordSize > 1) {
		delete[]dRegInArray;
		delete[]dRegOutArray;
	}
}				// ~TapActionDRScan ()

//! Process the Shift-DR action

//! This drives the DR-Scan state. We can only do this if we have the TAP
//! state machine in a consistent state, which in turn is only possible if we
//! have been through a reset cycle.

//! If the state machine shows it has yet to be through a reset cycle, we
//! drive that cycle, after issuing a warning. This functionality is provided
//! by the parent class, TapAction::.

//! @param[in]  tapStateMachine  The TAP state machine with which this action
//!                              is associated. 
//! @param[out] tdi              The value to drive on TDI
//! @param[in]  tdo              The value currently on TDO
//! @param[out] tms              The value to drive on TMS

//! @return  True if the action is complete

bool TapActionDRScan::process(TapStateMachine * tapStateMachine,
			      bool & tdi, bool tdo, bool & tms)
{
	// Ensure we are in a consistent state. If not then we'll have moved towards
	// it and can return with the given tms
	if (!checkResetDone(tapStateMachine, tms, true)) {
		return false;
	}

	if (goToPauseState) {
		switch (dRScanState) {
		case SHIFT_DR_PREPARING:

			// Are we in the Shift-DR state yet?
			if (!tapStateMachine->targetState(TAP_SHIFT_DR, tms)) {
				return false;	// Not there. Accept the TMS value
			} else {
				dRScanState = SHIFT_DR_SHIFTING_BEFORE_PAUSE;	// Drop through
			}

		case SHIFT_DR_SHIFTING_BEFORE_PAUSE:

			// Are we still shifting stuff?
			if (bitsShifted < bitsBeforePause) {
				// We are in the Shift-DR state. Another bit about to be done, so
				// increment the count
				bitsShifted++;

				// Set the TDI value. In a routine to keep this tidy.
				tdi = shiftDRegOut();

				// Record the TDO value. This is always a cycle late, so we ignore
				// it the first time. The value shifts in from the top.
				if (bitsShifted > 1) {
					shiftDRegIn(tdo);
				}
				// TMS is 0 to keep us here UNLESS this is the last bit, in which case
				// it is 1 to move us into Exit1-DR.
				tms = (bitsShifted == bitsBeforePause);

				// Not done until we've updated
				return false;
			} else {
				// Capture the last TDO bit
				shiftDRegIn(tdo);

				// Now lower TMS to go to PAUSE_DR
				tms = false;

				dRScanState = SHIFT_DR_SHIFTING_PAUSE;

			}

		case SHIFT_DR_SHIFTING_PAUSE:
			{
				if (!tapStateMachine->targetState
				    (TAP_PAUSE_DR, tms)) {
					return false;	// Not there. Accept the TMS value
				}

				if (pauseStateCount++ < 3)
					return false;
				// Sit in DR_PAUSE state and cycle until TDO is low
				// tms starts false, should get set to true on the cycle
				// tdo goes low, then the next cycle we go back to SHIFT_DR
				// and we return so tms isn't set again.
				if (!tdo) {
					tms = true;
					dRScanState = SHIFT_DR_EXIT2;
					return false;
				}
			}

		case SHIFT_DR_EXIT2:
			{
				tms = false;
				shiftDRegIn(0);
				dRScanState = SHIFT_DR_SHIFTING_AFTER_PAUSE;
				return false;

			}

		case SHIFT_DR_SHIFTING_AFTER_PAUSE:
			{
				if (bitsShifted < dRegBitSize) {
					// We are in the Shift-DR state. Another bit about to be done, so
					// increment the count
					bitsShifted++;

					// Set the TDI value. In a routine to keep this tidy.
					tdi = shiftDRegOut();

					//printf("shifting after pause (%d+32=%d) %d of %d tdo=%d\n",bitsBeforePause,bitsBeforePause+32, bitsShifted, dRegBitSize,(tdo) ? 1 : 0);
					shiftDRegIn(tdo);

					// TMS is 0 to keep us here UNLESS this is the last bit, in which case
					// it is 1 to move us into Exit1-DR.
					tms = (bitsShifted == dRegBitSize);

					// Not done until we've updated
					return false;
				} else {
					// Capture the last TDO bit
					shiftDRegIn(tdo);

					dRScanState = SHIFT_DR_UPDATING;	// Drop through
				}
			}

		case SHIFT_DR_UPDATING:

			// Are we still trying to update?
			if (!tapStateMachine->targetState(TAP_UPDATE_DR, tms)) {
				return false;	// Not there. Accept the TMS value
			} else {
				return true;	// All done
			}
		}
	} else {
		switch (dRScanState) {
		case SHIFT_DR_PREPARING:

			// Are we in the Shift-DR state yet?
			if (!tapStateMachine->targetState(TAP_SHIFT_DR, tms)) {
				return false;	// Not there. Accept the TMS value
			} else {
				dRScanState = SHIFT_DR_SHIFTING;	// Drop through
			}

		case SHIFT_DR_SHIFTING:

			// Are we still shifting stuff?
			if (bitsShifted < dRegBitSize) {
				// We are in the Shift-DR state. Another bit about to be done, so
				// increment the count
				bitsShifted++;

				// Set the TDI value. In a routine to keep this tidy.
				tdi = shiftDRegOut();

				// Record the TDO value. This is always a cycle late, so we ignore
				// it the first time. The value shifts in from the top.
				if (bitsShifted > 1) {
					shiftDRegIn(tdo);
				}
				// TMS is 0 to keep us here UNLESS this is the last bit, in which case
				// it is 1 to move us into Exit1-DR.
				tms = (bitsShifted == dRegBitSize);

				// Not done until we've updated
				return false;
			} else {
				// Capture the last TDO bit
				shiftDRegIn(tdo);

				dRScanState = SHIFT_DR_UPDATING;	// Drop through

			}

		case SHIFT_DR_UPDATING:

			// Are we still trying to update?
			if (!tapStateMachine->targetState(TAP_UPDATE_DR, tms)) {
				return false;	// Not there. Accept the TMS value
			} else {
				return true;	// All done
			}
		}
	}
}				// process ()

//! Get the shifted out value.

//! This version works with large values.

//! @param[out] dRegArray  Array for the result
void TapActionDRScan::getDRegOut(uint64_t dRegArray[])
{
	if (1 == dRegWordSize) {
		dRegArray[0] = dRegOut;
	} else {
		for (int i = 0; i < dRegWordSize; i++) {
			dRegArray[i] = dRegOutArray[i];
		}
	}
}				// getDRegOut ()

//! Get the shifted out value.

//! This version is for small values. For large values it silently returns the
//! bottom 64 bits only.

//! @todo  Should we give an error. Or is it useful to allow efficient access
//!        to the bottom 64 bits?

//! @return  The value shifted out (or the bottom 64 bits thereof if the
//!          register is "large").
uint64_t TapActionDRScan::getDRegOut()
{
	if (1 == dRegWordSize) {
		return dRegOut;
	} else {
		return dRegOutArray[0];
	}
}				// getDRegOut ()

//! Utility to shift the bottom bit out of the dReg.

//! Two flavours depending on whether we have a "small" register

//! @return  The bit shifted out.
bool TapActionDRScan::shiftDRegOut()
{
	if (1 == dRegWordSize)	// "Small" register
	{
		bool res = dRegIn & 1;
		dRegIn >>= 1;
		return res;
	} else			// "Large" register
	{
		bool res = (dRegInArray[0] & 1) == 1;

		// Shift all but the first word along
		for (int i = 0; i < (dRegWordSize - 1); i++) {
			dRegInArray[i] =
			    (dRegInArray[i] >> 1) | (dRegInArray[i + 1] << 63);
		}

		// Shift the first word
		dRegInArray[dRegWordSize - 1] >>= 1;

		return res;
	}
}				// shiftDRegOut ()

//! Utility to shift the top bit into the dReg.

//! Two flavours depending on whether we have a "small" register

//! @param bitIn  The bit to shift in the top
void TapActionDRScan::shiftDRegIn(bool bitIn)
{
	if (1 == dRegWordSize)	// "Small" register
	{
		dRegOut >>= 1;	// Move all the existing bits right

		if (bitIn)	// OR any new bit in
		{
			uint64_t tmpBit = 1ULL << (dRegBitSize - 1);
			dRegOut |= tmpBit;
		}
	} else			// "Large" register
	{
		// Shift all but the first word along
		for (int i = 0; i < (dRegWordSize - 1); i++) {
			dRegOutArray[i] >>= 1;
			dRegOutArray[i] |= dRegOutArray[i + 1] << 63;
		}

		// The first word is shifted and the new bit masked in
		dRegOutArray[dRegWordSize - 1] >>= 1;
		dRegOutArray[dRegWordSize - 1] |=
		    bitIn ? (topMask + 1) >> 1 : 0;
	}
}				// shiftDRegIn ()
