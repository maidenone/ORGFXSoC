// ----------------------------------------------------------------------------

// SystemC OpenRISC 1000 Debug Unit: definition

// Copyright (C) 2008  Embecosm Limited <info@embecosm.com>

// Contributor Jeremy Bennett <jeremy.bennett@embecosm.com>

// This file is part of the GDB interface to the cycle accurate model of the
// OpenRISC 1000 based system-on-chip, ORPSoC, built using Verilator.

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

#ifndef DEBUG_UNIT_SC__H
#define DEBUG_UNIT_SC__H

// Define if no cache is wanted
#define NOCACHE

#include <stdint.h>

#include "systemc"

#include "JtagSC_includes.h"
#include "OrpsocAccess.h"
#include "SprCache.h"
#include "MemCache.h"

//-----------------------------------------------------------------------------
//! Module modeling the OpenRISC 1000 Debug Unit

//! Provides a high level interface to the GDB Server module with functions to
//! access SPRs, Wishbone memory and CPU control.

//! Provides a low level interface to the Embecosm SystemC JTAG interface,
//! queueing requests to read and write JTAG registers.
//-----------------------------------------------------------------------------
class DebugUnitSC:public sc_core::sc_module {
public:

	// Constructor and destructor
	DebugUnitSC(sc_core::sc_module_name name,
		    sc_core::sc_fifo < TapAction * >*_tapActionQueue);
	~DebugUnitSC();

	// Reset function for the debug unit
	void resetDebugUnit();

	// Functions to control and report on the CPU
	void reset();
	void stall();
	void unstall();
	bool isStalled();

	// Functions to access SPRs
	uint32_t readSpr(uint16_t sprNum);
	void writeSpr(uint16_t sprNum, uint32_t value);
	void andSpr(uint16_t sprNum, uint32_t value);
	void orSpr(uint16_t sprNum, uint32_t value);

	// Functions to access memory
	uint32_t readMem32(uint32_t addr);
	bool writeMem32(uint32_t addr, uint32_t value);
	uint8_t readMem8(uint32_t addr);
	bool writeMem8(uint32_t addr, uint8_t value);

private:

	// JTAG instructions
	static const uint32_t CHAIN_SELECT_IR = 0x3;	//!< Chain Select instruction
	static const uint32_t DEBUG_IR = 0x8;	//!< Debug instruction

	//! JTAG instruction register length. There is no CRC for this register.
	static const int JTAG_IR_LEN = 4;	//!< JTAG instr reg length

	// DEBUG UNIT CHAIN data register fields
	static const int DUSEL_DR_LEN = 73;	//!< total DUSEL DR size
	static const int DUSEL_SEL_OFF = 0;	//!< start of select field
	static const int DUSEL_SEL_LEN = 1;	//!< length of select field
	static const int DUSEL_OPCODE_OFF = DUSEL_SEL_OFF + DUSEL_SEL_LEN;	//!< start of opcode field
	static const int DUSEL_OPCODE_LEN = 4;	//!< length of opcode field
	static const int DUSEL_CRC_OFF = DUSEL_OPCODE_OFF + DUSEL_OPCODE_LEN;	//!< start of CRC field
	static const int DUSEL_CRC_LEN = 32;	//!< length of CRC field
	static const int DUSEL_RESP_STATUS_OFF = DUSEL_CRC_OFF + DUSEL_CRC_LEN;
	static const int DUSEL_RESP_STATUS_LEN = 4;
	static const int DUSEL_RESP_CRC_OFF =
	    DUSEL_RESP_STATUS_OFF + DUSEL_RESP_STATUS_LEN;
	static const int DUSEL_RESP_CRC_LEN = 32;

	static const uint32_t DBG_CRC32_POLY = 0x04c11db7;

	// OpenRISC 1000 scan chains (values in DUSEL data register field)
	static const int OR1K_SC_UNDEF = -1;	//!< Undefined OR1K scan chain
	static const int OR1K_SC_WISHBONE = 0;	//!< for memory access
	static const int OR1K_SC_CPU0 = 1;	//!< for access to CPU0
	static const int OR1K_SC_CPU1 = 2;	//!< for access to CPU1

	// JTAG RISC_DEBUG (for accessing SPR) data register fields
	static const int RISC_DEBUG_DR_LEN = 74;	//!< Total RISC_DEBUG DR size
	static const int RISC_DEBUG_ADDR_OFF = 0;	//!< start of address field
	static const int RISC_DEBUG_ADDR_LEN = 32;	//!< length of address field
	static const int RISC_DEBUG_RW_OFF = 32;	//!< start of read/write field
	static const int RISC_DEBUG_RW_LEN = 1;	//!< length of read/write field
	static const int RISC_DEBUG_DATA_OFF = 33;	//!< start of data field
	static const int RISC_DEBUG_DATA_LEN = 32;	//!< length of data field
	static const int RISC_DEBUG_CRC_OFF = 65;	//!< start of CRC field
	static const int RISC_DEBUG_CRC_LEN = 8;	//!< length of CRC field
	static const int RISC_DEBUG_SPARE_OFF = 73;	//!< start of spare bits
	static const int RISC_DEBUG_SPARE_LEN = 1;	//!< length of spare bit field

	// JTAG REGISTER (for controlling the CPU) data register fields
	static const int REGISTER_DR_LEN = 47;	//!< Total REGISTER DR size
	static const int REGISTER_ADDR_OFF = 0;	//!< start of address field
	static const int REGISTER_ADDR_LEN = 5;	//!< length of address field
	static const int REGISTER_RW_OFF = 5;	//!< start of read/write field
	static const int REGISTER_RW_LEN = 1;	//!< length of read/write field
	static const int REGISTER_DATA_OFF = 6;	//!< start of data field
	static const int REGISTER_DATA_LEN = 32;	//!< length of data field
	static const int REGISTER_CRC_OFF = 38;	//!< start of CRC field
	static const int REGISTER_CRC_LEN = 8;	//!< length of CRC field
	static const int REGISTER_SPARE_OFF = 46;	//!< start of spare bits
	static const int REGISTER_SPARE_LEN = 1;	//!< length of spare bit field

	// Register addresses for the REGISTER scan chain
	static const uint8_t OR1K_RSC_RISCOP = 0x04;	//!< Used to reset/stall CPU

	// Bits for the RISCOP register
	static const uint32_t RISCOP_RESET = 0x00000001;	//!< Reset the CPU
	static const uint32_t RISCOP_STALL = 0x00000002;	//!< Stall the CPU

	// JTAG WISHBONE (for accessing SPR) data register fields
	static const int WISHBONE_DR_LEN = 74;	//!< Total WISHBONE DR size
	static const int WISHBONE_ADDR_OFF = 0;	//!< start of address field
	static const int WISHBONE_ADDR_LEN = 32;	//!< length of address field
	static const int WISHBONE_RW_OFF = 32;	//!< start of read/write field
	static const int WISHBONE_RW_LEN = 1;	//!< length of read/write field
	static const int WISHBONE_DATA_OFF = 33;	//!< start of data field
	static const int WISHBONE_DATA_LEN = 32;	//!< length of data field
	static const int WISHBONE_CRC_OFF = 65;	//!< start of CRC field
	static const int WISHBONE_CRC_LEN = 8;	//!< length of CRC field
	static const int WISHBONE_SPARE_OFF = 73;	//!< start of spare bits
	static const int WISHBONE_SPARE_LEN = 1;	//!< length of spare bit field

	//! The NPC is special, so we need to know about it
	static const int SPR_NPC = 0x10;

	//! The JTAG fifo we queue on
	sc_core::sc_fifo < TapAction * >*tapActionQueue;

	//! The processor stall state. When stalled we can use cacheing on
	//! reads/writes of memory and SPRs.
	enum {
		UNKNOWN,
		STALLED,
	} stallState;

	//! The currently selected scan chain
	int currentScanChain;

#ifdef NOCACHE
	//! Even if no cached, we need to cache the NPC
	uint32_t npcCachedValue;

	//! Cached NPC is valid
	bool npcCacheIsValid;

#else
	//! The SPR cache
	SprCache *sprCache;

	//! The memory cache
	MemCache *memCache;
#endif

	// Functions to control the CPU
	uint32_t readRiscop();
	void writeRiscop(uint32_t value);

	// Or1k JTAG actions
	void selectDebugModule(int chain);
	uint32_t readJtagReg(uint32_t addr);
	uint32_t readJtagReg1(uint32_t addr, int bitSizeNoCrc);
	uint32_t readJtagReg1(uint64_t * dRegArray,
			      uint32_t addr, int bitSizeNoCrc);
	void writeJtagReg(uint32_t addr, uint32_t data);

	// Utilities to pack and unpack bits to/from data registers.
	void clearBits(uint64_t regArray[], int regBits);

	void packBits(uint64_t regArray[],
		      int fieldOffset, int fieldBits, uint64_t fieldVal);

	uint64_t unpackBits(uint64_t regArray[],
			    int fieldOffset, int fieldBits);

	// Utility to compute CRC-8 the OpenRISC way.
	uint8_t crc8(uint64_t dataArray[], int size);

	// Utility to compute CRC-32 for the debug unit
	uint32_t crc32(uint64_t dataArray[], int size, int offset);

	// Functions to bitreverse values
	uint32_t bit_reverse_swar_2(uint32_t x);
	uint32_t bit_reverse_swar_4(uint32_t x);
	uint32_t bit_reverse_swar_8(uint32_t x);
	uint32_t bit_reverse_swar_16(uint32_t x);
	uint32_t bit_reverse_swar_32(uint32_t x);
#define BITREV(x,y) bit_reverse_data(x,y)
	uint32_t bit_reverse_data(uint32_t x, int length);

};				// DebugUnitSC ()

#endif // DEBUG_UNIT_SC__H
