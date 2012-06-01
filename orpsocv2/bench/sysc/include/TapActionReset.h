// ----------------------------------------------------------------------------

// TAP reset action : definition

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

#ifndef TAP_RESET_ACTION__H
#define TAP_RESET_ACTION__H

#include "TapAction.h"
#include "TapStateMachine.h"

//! Class to represent a TAP reset action.

//! This can be very simple, since it reuses the parent class method to do the
//! reset.

class TapActionReset:public TapAction {
public:

	// Constructor
	TapActionReset(sc_core::sc_event * _doneEvent);

protected:

	// Process the action for reset
	bool process(TapStateMachine * tapStateMachine,
		     bool & tdi, bool tdo, bool & tms);

private:

	//!< Flag to mark first call to process method
	bool firstTime;

};				// TapActionReset

#endif // TAP_RESET_ACTION__H
