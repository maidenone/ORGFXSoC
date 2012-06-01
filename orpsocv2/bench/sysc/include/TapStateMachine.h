// ----------------------------------------------------------------------------

// The TAP state machine: definition

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

// ----------------------------------------------------------------------------

// $Id$

#ifndef TAP_STATE_MACHINE__H
#define TAP_STATE_MACHINE__H

//! Enumeration of all the states in the TAP state machine.

//! Last entry is not a state, but a marker of the number of states. Useful
//! for state transition matrices.
enum TapState {
	TAP_TEST_LOGIC_RESET = 0,
	TAP_RUN_TEST_IDLE = 1,
	TAP_SELECT_DR_SCAN = 2,
	TAP_CAPTURE_DR = 3,
	TAP_SHIFT_DR = 4,
	TAP_EXIT1_DR = 5,
	TAP_PAUSE_DR = 6,
	TAP_EXIT2_DR = 7,
	TAP_UPDATE_DR = 8,
	TAP_SELECT_IR_SCAN = 9,
	TAP_CAPTURE_IR = 10,
	TAP_SHIFT_IR = 11,
	TAP_EXIT1_IR = 12,
	TAP_PAUSE_IR = 13,
	TAP_EXIT2_IR = 14,
	TAP_UPDATE_IR = 15,
	TAP_SIZE = 16
};				// enum TapState

//! Textual representation of all the TAP states.

//! Provided for debugging purposes
static const char *tapStateNames[TAP_SIZE] = {
	"Test-Logic-Reset",
	"Run-Test/Idle",
	"Select-DR-Scan",
	"Capture-DR",
	"Shift-DR",
	"Exit1-DR",
	"Pause-DR",
	"Exit2-DR",
	"Update-DR",
	"Select-IR-Scan",
	"Capture-IR",
	"Shift-IR",
	"Exit1-IR",
	"Pause-IR",
	"Exit2-IR",
	"Update-IR"
};				// tapStateNames

//! TAP state machine

//! Tracks the state of the TAP. This should mirror the state of the TAP in
//! the connected HW.

//! The state machine is created in the reset condition, but in truth we
//! cannot know what the state is. It is essential the TAP is reset before
//! first being used.

//! We cannot know for certain when the TAP state machine has been reset. 5
//! consecutive TMS=1 transitions will take you there, but a reset of the
//! target could undo this. It is the responsibility of the user of the TAP
//! state machine.

//! For convenience of users, this class provides a flag (resetDone::),
//! with accessors by which reset state can be recorded.

class TapStateMachine {
public:

	friend class JtagSC;
	friend class TapAction;
	friend class TapActionDRScan;
	friend class TapActionIRScan;
	friend class TapActionReset;

protected:

	//! The number of cycles of TMS=1 required to force reset
	static const int TAP_RESET_CYCLES = 5;

	// Constructor
	 TapStateMachine();

	// Accessor for TAP state
	TapState getState();

	// Accessors for TAP reset state
	bool getResetDone();
	void setResetDone(bool _resetState);

	// Drive the TAP state machine
	void nextState(bool tms);

	// Determine if we are in a particular target state
	bool targetState(TapState target, bool & tms);

private:

	//! The current TAP state
	 TapState state;

	//! True if the TAP state machine has been through a reset.

	//! The state can be sure to match that of the target. Responsibility of
	//! user classes to set this.
	bool resetDone;

};				// class TapStateMachine

#endif // TAP_STATE_MACHINE__H
