// ----------------------------------------------------------------------------

// TAP action header: abstract class implementation

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

#include "TapAction.h"

class sc_event;

//! Constructor

//! Records the SystemC event used to notify completion and sets the
//! resetCounter to zero.

//! @param _actionType  The action type

TapAction::TapAction(sc_core::sc_event * _doneEvent):
doneEvent(_doneEvent), resetCounter(0)
{

}				// TapAction ()

//! Accessor to get the SystemC completion event

//! @return  The SystemC completion event

sc_core::sc_event * TapAction::getDoneEvent()
{
	return doneEvent;

}				// getDoneEvent ()

//! Function to check the TAP is in a consistent state, optionally with a
//! warning.

//! This is a convenience for subclasses (hence protected), so they can ensure
//! the state machine is in a consistent state.

//! @note The method returns TRUE to indicate that the machine is in the
//!       consistent state. However the TapStateMachine resetDone flag is set
//!       when the final TMS is set - the state will only be reached IF that
//!       TMS is driven. This mechanism gives users flexibility. They can
//!       detect the final TMS being driven.

//! @param[in]  tapStateMachine  The TAP state machine with which we are
//!                              associated.
//! @param[out] tms              The value to drive on the JTAG TMS
//! @param[in]  warn             True to indicate a warning message should be
//!                              issued when starting a reset cycle.

//! @return  TRUE if the TAP state machine was already in a consistent state.

bool TapAction::checkResetDone(TapStateMachine * tapStateMachine,
			       bool & tms, bool warn)
{
	// Nothing more to do if we are consistent
	if (tapStateMachine->getResetDone()) {
		return true;
	}
	// Need to reset. If requested and this is the first cycle of reset, give a
	// warning.
	if (warn && (0 == resetCounter)) {
		std::cerr << "JTAG TAP state inconsistent: resetting" <<
		    std::endl;
	}
	// Drive towards reset
	resetCounter++;
	tms = 1;

	// If we have got to the end of the reset sequence we can clear the
	// tapStateMachine and report we are consistent. However we will not return
	// true until the next call.
	if (tapStateMachine->TAP_RESET_CYCLES == resetCounter) {
		tapStateMachine->setResetDone(true);
		resetCounter = 0;	// Ready for next time
	} else {
		return false;
	}
}				// checkResetDone ()
