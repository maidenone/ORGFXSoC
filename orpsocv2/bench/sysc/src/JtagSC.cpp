// ----------------------------------------------------------------------------

// Main module providing the JTAG interface: implementation

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

#include "JtagSC.h"

SC_HAS_PROCESS(JtagSC);

//! Constructor for the JTAG handler.

//! @param[in] name       Name of this module, passed to the parent
//!                       constructor. 
//! @param[in] fifo_size  Size of the FIFO on which to queue TAP actions.

JtagSC::JtagSC(sc_core::sc_module_name name, int fifo_size):
sc_module(name), currentTapAction(NULL)
{
	tapActionQueue = new sc_core::sc_fifo < TapAction * >(fifo_size);
	stateMachine = new TapStateMachine();

	SC_METHOD(processActions);
	sensitive << tck.pos();

}				// JtagSC ()

//! Destructor for the JTAG handler.

//! Give up our state machine and FIFO

JtagSC::~JtagSC()
{
	delete stateMachine;
	delete tapActionQueue;

}				// ~JtagSC ()

//! Method to drive the jtag ports.

//! Initial version just drives the reset.

void
 JtagSC::processActions()
{
	// TRST is driven as the inverse of the system reset
	trst = !sysReset;

	// Do nothing else if in CPU reset (active high)
	if (sysReset) {
		return;
	}
	// Functions setting the outputs will need bools (they are not generally
	// SystemC modules, so don't handle the likes of sc_in<> correctly).
	bool tdi_o;
	bool tms_o;

	// Try to get an action if we don't have one
	if (NULL == currentTapAction) {
		if (false == tapActionQueue->nb_read(currentTapAction)) {
			// Nothing there, so head for Run-Test/Idle state.
			stateMachine->targetState(TAP_RUN_TEST_IDLE, tms_o);
			tms = tms_o;

			return;
		}
	}
	// Process the action, notifying the originator when done.

	if (currentTapAction->process(stateMachine, tdi_o, tdo, tms_o)) {
		currentTapAction->getDoneEvent()->notify();
		currentTapAction = NULL;
	}
	// Select the new TAP state
	stateMachine->nextState(tms_o);

	// Drive the signal ports
	tdi = tdi_o;
	tms = tms_o;

}				// processActions()
