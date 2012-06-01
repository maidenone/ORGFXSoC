// ----------------------------------------------------------------------------

// The TAP state machine: implementation

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

// The C/C++ parts of this program are commented throughout in a fashion
// suitable for processing with Doxygen.

// ----------------------------------------------------------------------------

// $Id$

#include "TapStateMachine.h"

//! Constructor

//! Start in the Test-Logic-Reset state, although we cannot know this reflects
//! the hardware until it has been through a TAP reset sequence. This is
//! reflected in the ::tapResetDone flag.

TapStateMachine::TapStateMachine():
state(TAP_TEST_LOGIC_RESET), resetDone(false)
{

}				// TapStateMachine ()

//! Accessor to get the current TAP state

//! Only guaranteed to match the target hardware if it has been through a
//! reset sequence.

//! @return  The current TAP state
TapState TapStateMachine::getState()
{
	return state;

}				// getState ()

//! Accessor to get the current TAP reset state.

//! It is the responsibility of classes using this class to correctly set this
//! state.

//! @return  The current TAP reset state
bool TapStateMachine::getResetDone()
{
	return resetDone;

}				// getResetDone ()

//! Accessor to set the current TAP reset state.

//! It is the responsibility of classes using this class to correctly set this
//! state.

//! @param[in]  The desired TAP reset state
void TapStateMachine::setResetDone(bool _resetDone)
{
	resetDone = _resetDone;

}				// setResetDone ()

//! Drive the TAP state machine

//! @param tms       The JTAG TMS pin
void TapStateMachine::nextState(bool tms)
{
	static const TapState mapHigh[TAP_SIZE] = {	// When TMS = 1/true          
		TAP_TEST_LOGIC_RESET,	// from TAP_TEST_LOGIC_RESET  
		TAP_SELECT_DR_SCAN,	// from TAP_RUN_TEST_IDLE     
		TAP_SELECT_IR_SCAN,	// from TAP_SELECT_DR_SCAN    
		TAP_EXIT1_DR,	// from TAP_CAPTURE_DR        
		TAP_EXIT1_DR,	// from TAP_SHIFT_DR          
		TAP_UPDATE_DR,	// from TAP_EXIT1_DR          
		TAP_EXIT2_DR,	// from TAP_PAUSE_DR          
		TAP_UPDATE_DR,	// from TAP_EXIT2_DR          
		TAP_SELECT_DR_SCAN,	// from TAP_UPDATE_DR         
		TAP_TEST_LOGIC_RESET,	// from TAP_SELECT_IR_SCAN    
		TAP_EXIT1_IR,	// from TAP_CAPTURE_IR        
		TAP_EXIT1_IR,	// from TAP_SHIFT_IR          
		TAP_UPDATE_IR,	// from TAP_EXIT1_IR          
		TAP_EXIT2_IR,	// from TAP_PAUSE_IR          
		TAP_UPDATE_IR,	// from TAP_EXIT2_IR          
		TAP_SELECT_DR_SCAN
	};			// from TAP_UPDATE_IR         

	static const TapState mapLow[TAP_SIZE] = {	// When TMS = 0/false
		TAP_RUN_TEST_IDLE,	// from TAP_TEST_LOGIC_RESET
		TAP_RUN_TEST_IDLE,	// from TAP_RUN_TEST_IDLE   
		TAP_CAPTURE_DR,	// from TAP_SELECT_DR_SCAN  
		TAP_SHIFT_DR,	// from TAP_CAPTURE_DR      
		TAP_SHIFT_DR,	// from TAP_SHIFT_DR        
		TAP_PAUSE_DR,	// from TAP_EXIT1_DR        
		TAP_PAUSE_DR,	// from TAP_PAUSE_DR        
		TAP_SHIFT_DR,	// from TAP_EXIT2_DR        
		TAP_RUN_TEST_IDLE,	// from TAP_UPDATE_DR       
		TAP_CAPTURE_IR,	// from TAP_SELECT_IR_SCAN  
		TAP_SHIFT_IR,	// from TAP_CAPTURE_IR      
		TAP_SHIFT_IR,	// from TAP_SHIFT_IR        
		TAP_PAUSE_IR,	// from TAP_EXIT1_IR        
		TAP_PAUSE_IR,	// from TAP_PAUSE_IR        
		TAP_SHIFT_IR,	// from TAP_EXIT2_IR        
		TAP_RUN_TEST_IDLE
	};			// from TAP_UPDATE_IR         

	state = tms ? mapHigh[state] : mapLow[state];

}				// nextState()

//! Determine if we are in a particular TAP state

//! Set TMS to get there optimally

//! @param[in]  target  The desired TAP state
//! @param[out] tms     Value of TMS to move towards the target state. Set
//!                     even if we are already in the state (in case we want
//!                     to loop).

//! @return  True if we are already in the target state
bool TapStateMachine::targetState(TapState target, bool & tms)
{
	// Map of the value of TMS which moves the state machine from the the state
	// in the row (first) to the state in the column (second)
	static const bool map[TAP_SIZE][TAP_SIZE] = {
		//  T  R  S  C  S  E  P  E  U  S  C  S  E  P  E  U 
		//  L  T  D  D  D  1  D  2  D  I  I  I  1  I  2  I
		//  R  I  R  R  R  D  R  D  R  R  R  R  I  R  I  R
		//        S        R     R     S        R     R
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	// map[TLR][x]
		{1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},	// map[RTI][x]
		{1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1},	// map[SDRS][x]
		{1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},	// map[CDR][x]
		{1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},	// map[SDR][x]
		{1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1},	// map[E1DR][x]
		{1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1},	// map[PDR][x]
		{1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1},	// map[E2DR][x]
		{1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},	// map[UDR][x]
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0},	// map[SIRS][x]
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1},	// map[CIR][x]
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1},	// map[SIR][x]
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1},	// map[E1IR][x]
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1},	// map[PIR][x]
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1},	// map[E2IR][x]
		{1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
	};			// map[UIR][x]

	tms = map[state][target];
	return state == target;

}				// targetState()
