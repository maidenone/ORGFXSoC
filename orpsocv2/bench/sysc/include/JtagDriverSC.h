// ----------------------------------------------------------------------------

// SystemC JTAG driver header

// Copyright (C) 2008  Embecosm Limited <info@embecosm.com>

// Contributor Jeremy Bennett <jeremy.bennett@embecosm.com>

// This file is part of the cycle accurate model of the OpenRISC 1000 based
// system-on-chip, ORPSoC, built using Verilator.

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

// $Id: JtagDriverSC.h 317 2009-02-22 19:52:12Z jeremy $

#ifndef JTAG_DRIVER_SC__H
#define JTAG_DRIVER_SC__H

#include <stdint.h>

#include "systemc"

#include "JtagSC_includes.h"

//! Module providing TAP requests to JTAG

//! Provides a queue of requests to the JTAG interface. Requests are at the
//! level of TAP actions for reset, DR-Scan or IR-Scan.

class JtagDriverSC:public sc_core::sc_module {
public:

	// Constructor
	JtagDriverSC(sc_core::sc_module_name name,
		     sc_core::sc_fifo < TapAction * >*_tapActionQueue);

private:

	// JTAG instructions
	static const uint32_t CHAIN_SELECT_IR = 0x3;	//!< Chain Select instruction
	static const uint32_t DEBUG_IR = 0x8;	//!< Debug instruction

	// JTAG register lengths (excluding CRC)
	static const int JTAG_IR_LEN = 4;	//!< JTAG instr reg length
	static const int CHAIN_DR_LEN = 4;	//!< Length of DR (excl CRC)
	static const int RISC_DEBUG_DR_LEN = 65;	//!< Length of DR (excl CRC)
	static const int REGISTER_DR_LEN = 38;	//!< Length of DR (excl CRC)
	static const int WISHBONE_DR_LEN = 65;	//!< Length of DR (excl CRC)

	// JTAG register address masks
	static const uint32_t RISC_DEBUG_ADDR_MASK = 0xffffffff;	//!< Mask for addr
	static const uint32_t REGISTER_ADDR_MASK = 0x0000001f;	//!< Mask for addr
	static const uint32_t WISHBONE_ADDR_MASK = 0xffffffff;	//!< Mask for addr

	// JTAG register R/W bit
	static const uint64_t RISC_DEBUG_RW = 0x100000000ULL;	//!< R/W bit mask
	static const uint64_t REGISTER_RW = 0x000000020ULL;	//!< R/W bit mask
	static const uint64_t WISHBONE_RW = 0x100000000ULL;	//!< R/W bit mask

	// JTAG register data field offsets
	static const int RISC_DEBUG_DATA_OFF = 33;	//!< Offset to data field
	static const int REGISTER_DATA_OFF = 6;	//!< Offset to data field
	static const int WISHBONE_DATA_OFF = 33;	//!< Offset to data field

	//! JTAG register data field sizes (all the same)
	static const int DR_DATA_LEN = 32;

	//! CRC length
	static const int CRC_LEN = 8;

	// OpenRISC 1000 scan chains
	static const int OR1K_SC_UNDEF = -1;	//!< Undefined OR1K scan chain
	static const int OR1K_SC_RISC_DEBUG = 1;	//!< for access to SPRs
	static const int OR1K_SC_REGISTER = 4;	//!< to stall/reset CPU
	static const int OR1K_SC_WISHBONE = 5;	//!< for memory access

	//! Register addresses for the REGISTER scan chain
	static const uint8_t OR1K_RSC_RISCOP = 0x04;	//!< Used to reset/stall CPU

	// Bits for the RISCOP register
	static const uint32_t RISCOP_STALL = 0x00000001;	//!< Stall the CPU
	static const uint32_t RISCOP_RESET = 0x00000002;	//!< Reset the CPU

	//! The JTAG fifo we queue on
	sc_core::sc_fifo < TapAction * >*tapActionQueue;

	//! The currently selected scan chain
	int currentScanChain;

	// SystemC thread to queue actions
	void queueActions();

	// Or1k JTAG actions
	void reset();
	void selectChain(int chain);
	uint32_t readReg(uint32_t addr);
	uint32_t readReg1(uint32_t addr, int bitSizeNoCrc);
	uint32_t readReg1(uint64_t * dRegArray,
			  uint32_t addr, int bitSizeNoCrc);
	void writeReg(uint32_t addr, uint32_t data);
	// Utilities to compute CRC-8 the OpenRISC way. Versions for "big" and
	// "small" numbers.
	uint8_t crc8(uint64_t data, int size);
	uint8_t crc8(uint64_t * dataArray, int size);

	// Utilities to insert and extract bit strings from vectors
	void insertBits(uint64_t str,
			int strLen, uint64_t * array, int startBit);
	uint64_t extractBits(uint64_t * array, int startBit, int strLen);

};				// JtagDriverSC ()

#endif // JTAG_DRIVER_SC__H
