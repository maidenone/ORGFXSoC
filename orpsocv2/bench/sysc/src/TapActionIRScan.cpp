// ----------------------------------------------------------------------------

// TAP IR-Scan action: implementation

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

#include "TapActionIRScan.h"
#include "TapStateMachine.h"

//! Constructor

//! Sets up the superclass with the SystemC completion event and initializes
//! our state as appropriate.

//! @param[in] doneEvent  SystemC event to be signalled when this action is
//!                       complete.
//! @param[in] _iRegIn    The register to shift in.
//! @param[in] _iRegSize  Size in bits of the register to shift in.

TapActionIRScan::TapActionIRScan(sc_core::sc_event * _doneEvent,
				 uint32_t _iRegIn,
				 int _iRegSize):TapAction(_doneEvent),
iRegIn(_iRegIn),
iRegSize(_iRegSize), iRegOut(0), bitsShifted(0), iRScanState(SHIFT_IR_PREPARING)
{

}				// TapActionIRScan ()

//! Process the Shift-IR action

//! This drives the IR-Scan state. We can only do this if we have the TAP
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

bool TapActionIRScan::process(TapStateMachine * tapStateMachine,
			      bool & tdi, bool tdo, bool & tms)
{
	// Ensure we are in a consistent state. If not then we'll have moved towards
	// it and can return with the given tms
	if (!checkResetDone(tapStateMachine, tms, true)) {
		return false;
	}
	// We are consistent, so work through the IR-Scan process
	switch (iRScanState) {
	case SHIFT_IR_PREPARING:

		// Are we in the Shift-IR state yet?
		if (!tapStateMachine->targetState(TAP_SHIFT_IR, tms)) {
			return false;	// Not there. Accept the TMS value
		} else {
			iRScanState = SHIFT_IR_SHIFTING;	// Drop through
		}

	case SHIFT_IR_SHIFTING:

		// Are we still shifting stuff?
		if (bitsShifted < iRegSize) {
			// We are in the Shift-IR state. Another bit about to be done, so
			// increment the count
			bitsShifted++;

			// Shift out the TDI value from the bottom of the register
			tdi = iRegIn & 1;
			iRegIn >>= 1;

			// Record the TDO value. This is always a cycle late, so we ignore
			// it the first time. The value shifts in from the top.
			if (bitsShifted > 1) {
				iRegOut >>= 1;	// Move all the existing bits right

				if (tdo)	// OR any new bit in
				{
					uint32_t tmpBit = 1 << (iRegSize - 1);
					iRegOut |= tmpBit;
				}
			}
			// TMS is 0 to keep us here UNLESS this is the last bit, in which
			// case it is 1 to move us into Exit1-IR.
			tms = (bitsShifted == iRegSize);

			return false;
		} else {
			// Capture the last TDO bit
			iRegOut >>= 1;	// Move all the existing bits right

			if (tdo)	// OR any new bit in
			{
				uint32_t tmpBit = 1 << (iRegSize - 1);
				iRegOut |= tmpBit;
			}

			iRScanState = SHIFT_IR_UPDATING;	// Drop through
		}

	case SHIFT_IR_UPDATING:

		// Are we still trying to update?
		if (!tapStateMachine->targetState(TAP_UPDATE_IR, tms)) {
			return false;	// Not there. Accept the TMS value
		} else {
			return true;	// All done
		}
	}
}				// process ()

//! Get the shifted out register

//! @return  The value of the shifted our register

uint32_t TapActionIRScan::getIRegOut()
{
	return iRegOut;

}				// getIRegOut ()
