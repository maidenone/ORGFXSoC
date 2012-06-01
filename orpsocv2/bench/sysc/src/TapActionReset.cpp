// ----------------------------------------------------------------------------

// TAP reset action : implementation

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

#include "TapActionReset.h"

//! Constructor

//! Records the SystemC completion event with the parent. Sets the ::firstTime
//! flag, so the ::process () method will mark the tapStateMachine as not
//! reset.

//! @param[in] _doneEvent  The SystemC completion event

TapActionReset::TapActionReset(sc_core::sc_event * _doneEvent):
TapAction(_doneEvent), firstTime(true)
{

}				// TapActionReset ()

//! Process the reset action

//! This reuses the parent class ::checkResetDone() method. The first time we
//! are called, we mark the state machine as being in an inconsistent state,
//! to force the reset.

//! We use the value of the TAP state machine's resetDone flag to trigger
//! completion, since this is set on the final reset cycle. The result from
//! ::checkResetDone () is only true on the first cycle AFTER reset.

//! @see TapAction::

//! @param[in]  tapStateMachine  The TAP state machine with which this action
//!                              is associated. 
//! @param[out] tdi              The value to drive on TDI
//! @param[in]  tdo              The value currently on TDO
//! @param[out] tms              The value to drive on TMS

//! @return  True if the action is complete

bool TapActionReset::process(TapStateMachine * tapStateMachine,
			     bool & tdi, bool tdo, bool & tms)
{
	if (firstTime) {
		tapStateMachine->setResetDone(false);
		firstTime = false;
	}
	// Parent does the work (no warning message). Our result draws on the value
	// set in the tapStateMachine, to avoid an extra cycle.
	checkResetDone(tapStateMachine, tms, false);

	return tapStateMachine->getResetDone();

}				// process ()
