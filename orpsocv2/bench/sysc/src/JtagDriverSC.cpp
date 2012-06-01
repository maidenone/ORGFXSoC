// ----------------------------------------------------------------------------

// SystemC JTAG driver

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

// $Id: JtagDriverSC.cpp 317 2009-02-22 19:52:12Z jeremy $

#include <iostream>
#include <iomanip>

#include "JtagDriverSC.h"

SC_HAS_PROCESS(JtagDriverSC);

//! Constructor for the JTAG driver.

//! We create a SC_THREAD in which we can spit out some actions. Must be a
//! thread, since we need to wait for the actions to complete.

//! @param[in] name             Name of this module, passed to the parent
//!                             constructor. 
//! @param[in] _tapActionQueue  Pointer to fifo of actions to perform

JtagDriverSC::JtagDriverSC(sc_core::sc_module_name name, sc_core::sc_fifo < TapAction * >*_tapActionQueue):
sc_module(name),
tapActionQueue(_tapActionQueue), currentScanChain(OR1K_SC_UNDEF)
{
	SC_THREAD(queueActions);

}				// JtagDriverSC ()

//! SystemC thread to queue some actions

//! Have to use a thread, since we will end up waiting for actions to
//! complete.

void JtagDriverSC::queueActions()
{
	uint32_t res;		// General result variable

	// Reset the JTAG
	reset();

	// Select the register scan chain to stall the processor, stall the
	// processor and check it has stalled
	selectChain(OR1K_SC_REGISTER);
	writeReg(OR1K_RSC_RISCOP, RISCOP_STALL);

	do {
		res = readReg(OR1K_RSC_RISCOP);
		std::cout << sc_core::sc_time_stamp().to_seconds() * 1000000
		    << "us: RISCOP = " << std::hex << res << std::endl;
	}
	while ((res & RISCOP_STALL) != RISCOP_STALL);

	// Write the NPC SPR. Select the RISC_DEBUG scan chain, read the
	// register, write the register and read it back.
	selectChain(OR1K_SC_RISC_DEBUG);

	res = readReg(0x10);	// NPC
	std::cout << sc_core::sc_time_stamp().to_seconds() * 1000000
	    << "us: Old NPC = " << std::hex << res << std::endl;

	writeReg(0x10, 0x4000100);

	res = readReg(0x10);	// NPC
	std::cout << sc_core::sc_time_stamp().to_seconds() * 1000000
	    << "us: New NPC = " << std::hex << res << std::endl;

	// Unstall and check it has unstalled
	selectChain(OR1K_SC_REGISTER);
	writeReg(OR1K_RSC_RISCOP, 0);

	do {
		res = readReg(OR1K_RSC_RISCOP);
		std::cout << sc_core::sc_time_stamp().to_seconds() * 1000000
		    << "us: RISCOP = " << std::hex << res << std::endl;
	}
	while ((res & RISCOP_STALL) == RISCOP_STALL);

}				// queueActions ()

//! Reset the JTAG

//! @note Must be called from a SystemC thread, because of the use of wait()

void JtagDriverSC::reset()
{
	sc_core::sc_event * actionDone = new sc_core::sc_event();
	TapActionReset *resetAction;

	// Create and queue the reset action and wait for it to complete
	resetAction = new TapActionReset(actionDone);
	tapActionQueue->write(resetAction);
	wait(*actionDone);

	delete resetAction;
	delete actionDone;

}				// reset ()

//! Select an OpenRISC 1000 scan chain

//! Built on top of the JTAG commands to shift registers
//! We only do something if the scan chain needs to be changed.
//! - Shift-IR the CHAIN_SELECT instruction
//! - Shift-DR the specified chain
//! - Shift-IR the DEBUG instruction

//! @note Must be called from a SystemC thread, because of the use of wait()

//! @param[in] chain  The desired scan chain

void JtagDriverSC::selectChain(int chain)
{
	if (chain == currentScanChain) {
		return;
	} else {
		currentScanChain = chain;
	}

	sc_core::sc_event * actionDone = new sc_core::sc_event();
	TapActionIRScan *iRScan;
	TapActionDRScan *dRScan;

	// Create and queue the IR-Scan action for CHAIN_SELECT (no CRC)
	iRScan = new TapActionIRScan(actionDone, CHAIN_SELECT_IR, JTAG_IR_LEN);
	tapActionQueue->write(iRScan);
	wait(*actionDone);

	delete iRScan;

	// Create and queue the DR-Scan action for the specified chain (which we
	// know will fit into 64 bits)
	uint64_t chainReg = crc8(chain, CHAIN_DR_LEN) << (CHAIN_DR_LEN) | chain;
	dRScan =
	    new TapActionDRScan(actionDone, chainReg, CHAIN_DR_LEN + CRC_LEN);
	tapActionQueue->write(dRScan);
	wait(*actionDone);

	delete dRScan;

	// Create and queue the IR-Scan action for DEBUG (no CRC)
	iRScan = new TapActionIRScan(actionDone, DEBUG_IR, JTAG_IR_LEN);
	tapActionQueue->write(iRScan);
	wait(*actionDone);

	delete iRScan;
	delete actionDone;

}				// selectChain()

//! Read an OpenRISC 1000 JTAG register

//! Built on top of the JTAG commands to shift registers
//! - Shift-DR the specified address with R/W field unset
//! - read out the data shifted out.

//! DR register fields depend on the scan chain in use. For SC_REGISTER:
//! -   [4:0] Address to read from
//! -     [5] 0 indicating read
//! -  [37:6] Unused
//! - [45:38] CRC (CRC-8-ATM)

//! For SC_RISC_DEBUG (i.e. SPRs) and SC_WISHBONE:
//! -  [31:0] Address to read from
//! -    [32] 0 indicating read
//! - [64:33] unused
//! - [72:65] CRC (CRC-8-ATM)

//! In general two Scan-DR loops are needed. The first will cause the value
//! associated with the address to be loaded into the shift register, the
//! second will actually shift that value out. So we use a subsidiary call to
//! do the read (::readReg1()). This allows a future extension, where a block
//! of registers are read efficiently by overlapping ScanDR actions.

//! We can also provide a variant of ::readReg1 () that is optimized for
//! "small" value.

//! @note Must be called from a SystemC thread, because of the use of wait()

//! @param[in] addr  The address of the register

//! @return  The register value read

uint32_t JtagDriverSC::readReg(uint32_t addr)
{
	bool firstTime = true;
	int bitSizeNoCrc;	// Size of reg w/o its CRC field

	// Determine the size of register to read.
	switch (currentScanChain) {
	case OR1K_SC_RISC_DEBUG:
		bitSizeNoCrc = RISC_DEBUG_DR_LEN;
		break;

	case OR1K_SC_REGISTER:
		bitSizeNoCrc = REGISTER_DR_LEN;
		break;

	case OR1K_SC_WISHBONE:
		bitSizeNoCrc = WISHBONE_DR_LEN;
		break;
	}

	// Read the register twice. Use an optimized version if the register is
	// "small".
	if ((bitSizeNoCrc + CRC_LEN) < 64) {
		(void)readReg1(addr, bitSizeNoCrc);
		return readReg1(addr, bitSizeNoCrc);
	} else {
		uint64_t *dReg =
		    new uint64_t[(bitSizeNoCrc + CRC_LEN + 63) / 64];
		(void)readReg1(dReg, addr, bitSizeNoCrc);
		uint32_t res = readReg1(dReg, addr, bitSizeNoCrc);
		delete[]dReg;

		return res;
	}
}				// readReg ()

//! Single read of an OpenRISC 1000 JTAG register

//! Built on top of the JTAG commands to shift registers
//! - Shift-DR the specified address with R/W field unset
//! - read out the data shifted out.

//! This version is for "small" values represented as a uint64_t.

//! @note Must be called from a SystemC thread, because of the use of wait()

//! @param[in]     addr          The address to read
//! @param[in]     bitSizeNoCrc  Size of the register excluding its CRC field

//! @return  The register value read

uint32_t JtagDriverSC::readReg1(uint32_t addr, int bitSizeNoCrc)
{
	// Useful fields and sizes and the register itself
	int fullBitSize = bitSizeNoCrc + CRC_LEN;
	int dataOffset = bitSizeNoCrc - DR_DATA_LEN;
	uint64_t dReg;

	// Allocate space for the shifted reg and a SystemC completion event
	sc_core::sc_event * actionDone = new sc_core::sc_event();

	// Loop until CRCs match
	while (true) {
		// Create the data to shift in
		dReg = 0ULL;
		dReg |= addr;
		uint8_t crc_in = crc8(dReg, bitSizeNoCrc);
		dReg |= (uint64_t) crc_in << bitSizeNoCrc;

		// Prepare the action, queue it and wait for it to complete
		TapActionDRScan *dRScan = new TapActionDRScan(actionDone, dReg,
							      fullBitSize);
		tapActionQueue->write(dRScan);
		wait(*actionDone);
		dReg = dRScan->getDRegOut();
		delete dRScan;

		// Check CRCs
		uint8_t crc_out = dReg >> bitSizeNoCrc;
		uint8_t crc_calc = crc8(dReg, bitSizeNoCrc);

		// All done if CRC matches
		if (crc_out == crc_calc) {
			delete actionDone;
			return (dReg >> dataOffset) & ((1ULL << DR_DATA_LEN) -
						       1);
		}
	}
}				// readReg1 ()

//! Single read of an OpenRISC 1000 JTAG register

//! Built on top of the JTAG commands to shift registers
//! - Shift-DR the specified address with R/W field unset
//! - read out the data shifted out.

//! This version is for "large" values represented as an array of uint64_t.

//! @note Must be called from a SystemC thread, because of the use of wait()

//! @param[in,out] dRegArray     The shift register to use
//! @param[in]     addr          The address to read
//! @param[in]     bitSizeNoCrc  Size of the register excluding its CRC field

//! @return  The register value read

uint32_t
    JtagDriverSC::readReg1(uint64_t * dRegArray,
			   uint32_t addr, int bitSizeNoCrc)
{
	// Useful fields and sizes
	int fullBitSize = bitSizeNoCrc + CRC_LEN;
	int dataOffset = bitSizeNoCrc - DR_DATA_LEN;

	// Allocate a SystemC completion event
	sc_core::sc_event * actionDone = new sc_core::sc_event();

	// Loop until CRCs match
	while (true) {
		// Create the data to shift in
		memset(dRegArray, 0, fullBitSize / 8);
		dRegArray[0] |= addr;
		uint8_t crc_in = crc8(dRegArray, bitSizeNoCrc);
		insertBits(crc_in, CRC_LEN, dRegArray, bitSizeNoCrc);

		// Prepare the action, queue it and wait for it to complete
		TapActionDRScan *dRScan =
		    new TapActionDRScan(actionDone, dRegArray,
					fullBitSize);
		tapActionQueue->write(dRScan);
		wait(*actionDone);
		dRScan->getDRegOut(dRegArray);
		delete dRScan;

		// Check CRCs
		uint8_t crc_out = extractBits(dRegArray, bitSizeNoCrc, CRC_LEN);
		uint8_t crc_calc = crc8(dRegArray, bitSizeNoCrc);

		// All done if CRC matches
		if (crc_out == crc_calc) {
			delete actionDone;
			return extractBits(dRegArray, dataOffset, DR_DATA_LEN);
		}
	}
}				// readReg1 ()

//! Write an OpenRISC 1000 JTAG register

//! Built on top of the JTAG commands to shift registers
//! - Shift-DR the specified address with R/W field set and data to write

//! DR register fields depend on the scan chain in use. For SC_REGISTER:
//! -   [4:0] Address to write to
//! -     [5] 1 indicating write
//! -  [37:6] Value to write
//! - [45:38] CRC (CRC-8-ATM)

//! For SC_RISC_DEBUG (i.e. SPRs) and SC_WISHBONE:
//! -  [31:0] Address to write to
//! -    [32] 1 indicating write
//! - [64:33] Value to write
//! - [72:65] CRC (CRC-8-ATM)

//! @note Must be called from a SystemC thread, because of the use of wait()

//! @param[in] addr  The address of the register
//! @param[in] data  The register data to write

void
 JtagDriverSC::writeReg(uint32_t addr, uint32_t data)
{
	int bitSizeNoCrc;	// Size of reg w/o its CRC field
	uint64_t writeBit;	// Mask for the write enable bit

	// Determine the size of register to write.
	switch (currentScanChain) {
	case OR1K_SC_RISC_DEBUG:
		bitSizeNoCrc = RISC_DEBUG_DR_LEN;
		writeBit = RISC_DEBUG_RW;
		break;

	case OR1K_SC_REGISTER:
		bitSizeNoCrc = REGISTER_DR_LEN;
		writeBit = REGISTER_RW;
		break;

	case OR1K_SC_WISHBONE:
		bitSizeNoCrc = WISHBONE_DR_LEN;
		writeBit = WISHBONE_RW;
		break;
	}

	// Create the register in an array
	int wordSize = (bitSizeNoCrc + CRC_LEN + 63) / 64;
	uint64_t *dReg = new uint64_t[wordSize];

	// Create the data to shift in
	memset(dReg, 0, wordSize * 8);
	dReg[0] |= writeBit | addr;
	insertBits(data, DR_DATA_LEN, dReg, bitSizeNoCrc - DR_DATA_LEN);
	insertBits(crc8(dReg, bitSizeNoCrc), CRC_LEN, dReg, bitSizeNoCrc);

	// Prepare the action, queue it and wait for it to complete
	sc_core::sc_event * actionDone = new sc_core::sc_event();
	TapActionDRScan *dRScan = new TapActionDRScan(actionDone, dReg,
						      bitSizeNoCrc + CRC_LEN);

	tapActionQueue->write(dRScan);
	wait(*actionDone);

	delete[]dReg;
	delete dRScan;
	delete actionDone;

}				// writeReg ()

//! Compute CRC-8-ATM

//! The data is in a uint64_t, for which we use the first size bits to compute
//! the CRC.

//! @Note I am using the same algorithm as the ORPSoC debug unit, but I
//!       believe its function is broken! I don't believe the data bit should
//!       feature in the computation of bits 2 & 1 of the new CRC.

//! @Note I've realized that this is an algorithm for LSB first, so maybe it
//!       is correct!

//! @param data  The data whose CRC is desired
//! @param size  The number of bits in the data

uint8_t JtagDriverSC::crc8(uint64_t data, int size)
{
	uint8_t crc = 0;

	for (int i = 0; i < size; i++) {
		uint8_t d = data & 1;	// Latest data bit
		data >>= 1;

		uint8_t oldCrc7 = (crc >> 7) & 1;
		uint8_t oldCrc1 = (crc >> 1) & 1;
		uint8_t oldCrc0 = (crc >> 0) & 1;
		uint8_t newCrc2 = d ^ oldCrc1 ^ oldCrc7;	// Why d?
		uint8_t newCrc1 = d ^ oldCrc0 ^ oldCrc7;	// Why d?
		uint8_t newCrc0 = d ^ oldCrc7;

		crc =
		    ((crc << 1) & 0xf8) | (newCrc2 << 2) | (newCrc1 << 1) |
		    newCrc0;
	}

	return crc;

}				// crc8 ()

//! Compute CRC-8-ATM

//! The data is in an array of uint64_t, for which we use the first size bits
//! to compute the CRC.

//! @Note I am using the same algorithm as the ORPSoC debug unit, but I
//!       believe its function is broken! I don't believe the data bit should
//!       feature in the computation of bits 2 & 1 of the new CRC.

//! @Note I've realized that this is an algorithm for LSB first, so maybe it
//!       is correct!

//! @param dataArray  The array of data whose CRC is desired
//! @param size       The number of bits in the data

uint8_t JtagDriverSC::crc8(uint64_t dataArray[], int size)
{
	uint8_t crc = 0;

	for (int i = 0; i < size; i++) {
		uint8_t d = (dataArray[i / 64] >> (i % 64)) & 1;
		uint8_t oldCrc7 = (crc >> 7) & 1;
		uint8_t oldCrc1 = (crc >> 1) & 1;
		uint8_t oldCrc0 = (crc >> 0) & 1;
		uint8_t newCrc2 = d ^ oldCrc1 ^ oldCrc7;	// Why d?
		uint8_t newCrc1 = d ^ oldCrc0 ^ oldCrc7;	// Why d?
		uint8_t newCrc0 = d ^ oldCrc7;

		crc =
		    ((crc << 1) & 0xf8) | (newCrc2 << 2) | (newCrc1 << 1) |
		    newCrc0;
	}

	return crc;

}				// crc8 ()

//! Utility to insert a string of bits into array

//! This is a simple overwriting

//! @param  str       Bits to insert
//! @param  strLen    Number of bits to insert
//! @param  array     Array into which to insert
//! @param  startBit  Offset at which to insert bits

void JtagDriverSC::insertBits(uint64_t str,
			      int strLen, uint64_t * array, int startBit)
{
	int startWord = startBit / 64;
	int endWord = (startBit + strLen - 1) / 64;

	startBit = startBit % 64;

	// Deal with the startWord. Get enough bits for the mask and put them in the
	// right place
	uint64_t startMask = ((1ULL << strLen) - 1ULL) << startBit;

	array[startWord] &= ~startMask;
	array[startWord] |= str << startBit;

	// If we were all in one word, we can give up now.
	if (startWord == endWord) {
		return;
	}
	// Deal with the endWord. Get enough bits for the mask. No need to shift
	// these up - they're always at the bottom of the word
	int bitsToDo = (startBit + strLen) % 64;

	uint64_t endMask = (1ULL << bitsToDo) - 1ULL;

	array[endWord] &= ~endMask;
	array[endWord] |= str >> (strLen - bitsToDo);

}				// insertBits()

//! Utility to extract a string of bits from an array

//! @param  array     Array from which to extract
//! @param  startBit  Offset at which to extract bits
//! @param  strLen    Number of bits to extract

//! @return  Extracted bits

uint64_t JtagDriverSC::extractBits(uint64_t * array, int startBit, int strLen)
{
	int startWord = startBit / 64;
	int endWord = (startBit + strLen - 1) / 64;

	startBit = startBit % 64;

	// Deal with the startWord. Get enough bits for the mask and put them in the
	// right place
	uint64_t startMask = ((1ULL << strLen) - 1ULL) << startBit;
	uint64_t res = (array[startWord] & startMask) >> startBit;

	// If we were all in one word, we can give up now.
	if (startWord == endWord) {
		return res;
	}
	// Deal with the endWord. Get enough bits for the mask. No need to shift
	// these up - they're always at the bottom of the word
	int bitsToDo = (startBit + strLen) % 64;
	uint64_t endMask = (1ULL << bitsToDo) - 1ULL;

	return res | ((array[endWord] & endMask) << (strLen - bitsToDo));

}				// extractBits ()
