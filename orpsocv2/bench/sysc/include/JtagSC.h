// ----------------------------------------------------------------------------

// Main module providing the JTAG interface: definition

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

#ifndef JTAG_SC__H
#define JTAG_SC__H

#include "systemc"
#include "TapAction.h"

//! Default size of the FIFO queuing TAP actions
#define DEFAULT_TAP_FIFO_SIZE  256

//! The main JTAG interface module

//! This provides a pin interface on one side (to drive JTAG ports on a chip)
//! and a FIFO on the other, allowing the user to queue JTAG actions.

//! The pin interface is:
//! - sysReset (input)  The system reset. When this is asserted, the interface
//!                     will do nothing except drive TRST low.
//! - tck (input)       The JTAG clock (supplied externally)
//! - tdi (output)      Drives the TDI pin
//! - tdo (input)       Receives the TDO pin
//! - tms (output)      Drives the TMS pin
//! - trst (output)     Drives the TRST pin

//! @note The JTAG pins are reversed, because this module is @b driving the
//!       pins.

//! The FIFO allows the user to queue actions, which are of abstract type
//! TapAction::. This is subclassed to provide specific actions for reset
//! (TapActionReset::), DR-scan (TapActionDRScan::) and IR-scan
//! (TapActionIRScan::).

//! The size of the FIFO can optionally be specified in the constructor.

//! The class provides a method, sensitive to the clock, which reads actions
//! from the FIFO and processes them, issuing the requisite sequence of
//! changes of the JTAG pins until the action is complete. It then notifies
//! the action owner of completion, through an sc_event, which is part of the
//! ::TapAction class.
//! 
class JtagSC:public sc_core::sc_module {
public:

	// The ports. Note that the naming of the low level JTAG ports is reversed,
	// because we are driving the inputs! */
	sc_core::sc_in < bool > sysReset;	//!< The system reset (active high)

	sc_core::sc_in < bool > tck;	//!< External JTAG TCK
	sc_core::sc_out < bool > tdi;	//!< JTAG TDI pin
	sc_core::sc_in < bool > tdo;	//!< JTAG TDO pin
	sc_core::sc_out < bool > tms;	//!< JTAG TMS pin
	sc_core::sc_out < bool > trst;	//!< JTAG TRST pin

	//! JTAG action queue
	sc_core::sc_fifo < TapAction * >*tapActionQueue;

	// Constructor and destructor
	JtagSC(sc_core::sc_module_name name,
	       int fifo_size = DEFAULT_TAP_FIFO_SIZE);
	~JtagSC();

protected:

	// Method to process the actions
	void processActions();

private:

	//! The TAP state machine
	TapStateMachine * stateMachine;

	//! The next TAP action
	TapAction *currentTapAction;

};				// JtagSC ()

#endif // JTAG_SC__H
