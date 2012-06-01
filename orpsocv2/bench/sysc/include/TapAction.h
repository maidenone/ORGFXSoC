// ----------------------------------------------------------------------------

// TAP action header: abstract class definition

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

#ifndef TAP_ACTION__H
#define TAP_ACTION__H

#include "TapStateMachine.h"

namespace sc_core {
	class sc_event;
}
//! Enumeration of all the TAP actions supported. 
enum TapActionType {
	TAP_ACTION_RESET = 0,
	TAP_ACTION_SHIFT_DR = 1,
	TAP_ACTION_SHIFT_IR = 2
};

//! Abstract class to represent a TAP action.

//! Subclasses implement specific actions: Reset (TapActionReset::), DR-Scan
//! (TapActionDRScan::) and IR-Scan (TapActionIRScan::).

//! We keep a SystemC event, which is used to notify the creator of
//! completion. Since we are not a SystemC class, we don't do the notification
//! ourselves.

class TapAction {
public:

	friend class JtagSC;

	// Constructor
	TapAction(sc_core::sc_event * _doneEvent);

protected:

	// Accessor for the SystemC event to notify completion
	sc_core::sc_event * getDoneEvent();

	// Process the action. Pure virtual, so must be implemented by subclasses.
	virtual bool process(TapStateMachine * tapStateMachine,
			     bool & tdi, bool tdo, bool & tms) = 0;

	// Function to drive the TAP to a consistent state, optionally with a
	// warning.
	bool checkResetDone(TapStateMachine * tapStateMachine,
			    bool & tms, bool warn = false);

private:

	//! The associated SystemC event to mark completion
	 sc_core::sc_event * doneEvent;

	//! Counter for the reset process
	int resetCounter;

};				// TapAction ()

#endif // TAP_ACTION__H
