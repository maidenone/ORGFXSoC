// ----------------------------------------------------------------------------

// SystemC OpenRISC 1000 Debug Unit: implementation

// Copyright (C) 2008  Embecosm Limited <info@embecosm.com>
// Copyright (C) 2009  ORSoC

// Contributor Jeremy Bennett <jeremy.bennett@embecosm.com>
// Contributor Julius Baxter <julius@orsoc.se>

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

#include <iostream>
#include <iomanip>

#include "DebugUnitSC.h"
#include "Utils.h"

using sc_core::sc_event;
using sc_core::sc_fifo;
using sc_core::sc_module_name;

//-----------------------------------------------------------------------------
//! Constructor for the Debug Unit

//! This module is entirely subservient to the GDB server. It has no SystemC
//! processes of its own. It provides services via calls to its API.

//! The current scan chain is marked as undefined, and the current JTAG scan
//! chain is maked as undefined.

//! Caches for SPR and memory access are initialized.

//! This makes use of the Embecosm cycle accurate SystemC JTAG interface.

//! @see Embecosm Application Note 5 "Using JTAG with SystemC: Implementation
//!      of a Cycle Accurate Interface"
//!      (http://www.embecosm.com/download/ean5.html)

//! @param[in] name             Name of this module, passed to the parent
//!                             constructor. 
//! @param[in] _tapActionQueue  Pointer to fifo of actions to be performed by
//!                             the JTAG interface
//-----------------------------------------------------------------------------
DebugUnitSC::DebugUnitSC(sc_module_name name, sc_fifo < TapAction * >*_tapActionQueue):
sc_module(name),
tapActionQueue(_tapActionQueue),
stallState(UNKNOWN), currentScanChain(OR1K_SC_UNDEF)
{
#ifdef NOCACHE
	npcCacheIsValid = false;	// Always cache NPC
#else
	sprCache = new SprCache();
	memCache = new MemCache();
#endif

}				// DebugUnitSC ()

//-----------------------------------------------------------------------------
//! Destructor

//! Free up data structures
//-----------------------------------------------------------------------------
DebugUnitSC::~DebugUnitSC()
{
#ifndef NOCACHE
	delete memCache;
	delete sprCache;
#endif

}				// ~DebugUnitSC

//-----------------------------------------------------------------------------
//! Reset the Debug Unit

//! This is just a reset of the JTAG. It is quite possible to reset the debug
//! unit without resetting the whole target.

//! @note Must be called from a SystemC thread, because of the use of wait()
//-----------------------------------------------------------------------------
void
 DebugUnitSC::resetDebugUnit()
{
	sc_event *done = new sc_event();
	TapActionReset *resetAction;

	// Create and queue the reset action and wait for it to complete
	resetAction = new TapActionReset(done);
	tapActionQueue->write(resetAction);
	wait(*done);

	delete resetAction;
	delete done;

}				// resetDebugUnit ()

//-----------------------------------------------------------------------------
//! Reset the processor

//! Read the RISCOP register, OR in the reset bit and write it back.

//! After reset, the processor is known to be unstalled.
//-----------------------------------------------------------------------------
void DebugUnitSC::reset()
{
	writeRiscop(readRiscop() | RISCOP_RESET);
	stallState = UNKNOWN;

}				// reset ()

//-----------------------------------------------------------------------------
//! Stall the processor

//! Read the RISCOP register, OR in the stall bit and write it back.
//-----------------------------------------------------------------------------
void DebugUnitSC::stall()
{
	writeRiscop( /*readRiscop () | */ RISCOP_STALL);
	stallState = STALLED;

}				// stall ()

//-----------------------------------------------------------------------------
//! Unstall the processor

//! Read the RISCOP register, AND out the stall bit and write it back. After
//! this the NPC cache will be invalid.

//! @note Don't be tempted to read back for confirmation. Single stepping
//!       will already have stalled the processor again!
//-----------------------------------------------------------------------------
void DebugUnitSC::unstall()
{
	writeRiscop( /*readRiscop () & ~RISCOP_STALL */ 0);
	stallState = UNKNOWN;

#ifdef NOCACHE
	npcCacheIsValid = false;	// Always cache NPC
#else
	// Clear the caches
	sprCache->clear();
	memCache->clear();
#endif

}				// unstall ()

//-----------------------------------------------------------------------------
//! Report if the processor is stalled.

//! A stalled processor cannot spontaneously "unstall", so if the stallState
//! flag is STALLED, that value is returned. Otherwise the target is
//! interrogated to determine the status.

//! @return  TRUE if the processor is known to be stalled
//-----------------------------------------------------------------------------
bool DebugUnitSC::isStalled()
{
	if (STALLED == stallState) {
		return true;
	}

	uint32_t riscop = readRiscop();
	/* For some reason the reset bit is skipped over somewhere, so we should
	   just get riscop = 1 if it's stalled */

	//stallState = (RISCOP_STALL == (riscop & RISCOP_STALL)) ? STALLED : UNKNOWN;
	stallState = riscop ? STALLED : UNKNOWN;

	return STALLED == stallState;

}				// isStalled ()

//-----------------------------------------------------------------------------
//! Read the value of an OpenRISC 1000 Special Purpose Register

//! First see if we have the value in the cache, and if so return
//! it. Otherwise, select the RISC_DEBUG scan chain and read from JTAG,
//! storing the result in the cache.

//! @param[in]  sprNum  The SPR to read

//! @return  The value of the SPR
//-----------------------------------------------------------------------------
uint32_t DebugUnitSC::readSpr(uint16_t sprNum)
{
	uint32_t cachedValue;

#ifdef NOCACHE
	// Always check NPC cache
	if ((STALLED == stallState) && (sprNum == SPR_NPC) && npcCacheIsValid) {
		return npcCachedValue;
	}
#else
	// Use any cached value if we are stalled.
	if ((STALLED == stallState) && sprCache->read(sprNum, cachedValue)) {
		return cachedValue;	// Already there, no more to do
	}
#endif

	// Read the value
	selectDebugModule(OR1K_SC_CPU0);
	cachedValue = readJtagReg(sprNum);

#ifdef NOCACHE
	// Always update the NPC cache
	if ((STALLED == stallState) && (sprNum == SPR_NPC)) {
		npcCachedValue = cachedValue;
		npcCacheIsValid = true;
	}
#else
	// Update the cache if we are stalled
	if (STALLED == stallState) {
		sprCache->write(sprNum, cachedValue, sprNum == SPR_NPC);
	}
#endif

	return cachedValue;

}				// readSpr ()

//-----------------------------------------------------------------------------
//! Write the value of an OpenRISC 1000 Special Purpose Register

//! First look to see if we are stalled and the value is cached. If the value
//! has not changed, then we need to no more. Otherwise cache the value prior
//! to writing it.

//! Select the RISC_DEBUG scan chain and write to JTAG

//! @param[in] sprNum  The SPR to write
//! @param[in] value   The value to write
//-----------------------------------------------------------------------------
void DebugUnitSC::writeSpr(uint16_t sprNum, uint32_t value)
{
#ifdef NOCACHE
	// Always cache the NPC
	if ((STALLED == stallState) && (SPR_NPC == sprNum)) {
		// Have we already cached this NPC value?
		if (npcCacheIsValid && (value == npcCachedValue)) {
			return;
		} else {
			npcCachedValue = value;
			npcCacheIsValid = true;
		}
	}
#else
	if (STALLED == stallState) {
		// Have we already cached this value?
		uint32_t cachedValue;
		if (sprCache->read(sprNum, cachedValue) &&
		    (value == cachedValue)) {
			return;	// Already there, no more to do
		} else {
			sprCache->write(sprNum, value, sprNum == SPR_NPC);
		}
	}
#endif

	// Write the SPR
	selectDebugModule(OR1K_SC_CPU0);
	writeJtagReg(sprNum, value);

}				// writeSpr ()

//-----------------------------------------------------------------------------
//! AND the contents of an SPR with a value

//! A convenience combination of read and write

//! @param[in] sprNum  The SPR to write
//! @param[in] value   The value to AND into the register
//-----------------------------------------------------------------------------
void DebugUnitSC::andSpr(uint16_t sprNum, uint32_t value)
{
	writeSpr(sprNum, readSpr(sprNum) & value);

}				// andSpr ()

//-----------------------------------------------------------------------------
//! OR the contents of an SPR with a value

//! A convenience combination of read and write

//! @param[in] sprNum  The SPR to write
//! @param[in] value   The value to OR into the register
//-----------------------------------------------------------------------------
void DebugUnitSC::orSpr(uint16_t sprNum, uint32_t value)
{
	writeSpr(sprNum, readSpr(sprNum) | value);

}				// orSpr ()

//-----------------------------------------------------------------------------
//! Read a 32-bit word from the OpenRISC 1000 memory

//! Select the WISHBONE scan chain, then write the register. The data is in
//! model endianness and passed on without modification.

//! @todo Provide code to check if the read was from a valid address.

//! @param[in] addr  The address to read from

//! @return  The 32-bit value read
//-----------------------------------------------------------------------------
uint32_t DebugUnitSC::readMem32(uint32_t addr)
{
	uint32_t cachedValue;

#ifndef NOCACHE
	// Use any cached value if we are stalled.
	if ((STALLED == stallState) && memCache->read(addr, cachedValue)) {
		return cachedValue;	// Already there, no more to do
	}
#endif

	// Read the value
	selectDebugModule(OR1K_SC_WISHBONE);
	cachedValue = readJtagReg(addr);

#ifndef NOCACHE
	// Update the cache if we are stalled
	if (STALLED == stallState) {
		memCache->write(addr, cachedValue);
	}
#endif

	return cachedValue;

}				// readMem32 ()

//-----------------------------------------------------------------------------
//! Write a 32-bit word to the OpenRISC 1000 memory

//! Select the WISHBONE scan chain, then write the register. The data is in
//! model endianness and passed on without modification.

//! @todo Provide code to check if the write was to a valid address.

//! @param[in] addr   The address to write to
//! @param[in] value  The 32-bit value to write

//! @return  True if the write was successful. For now all writes are
//           successful.
//-----------------------------------------------------------------------------
bool DebugUnitSC::writeMem32(uint32_t addr, uint32_t value)
{
#ifndef NOCACHE
	if (STALLED == stallState) {
		// Have we already cached this value?
		uint32_t cachedValue;
		if (memCache->read(addr, cachedValue) && (value == cachedValue)) {
			return true;	// Already there, no more to do
		} else {
			memCache->write(addr, value);	// Write for the future
		}
	}
#endif

	// Write the memory
	selectDebugModule(OR1K_SC_WISHBONE);
	writeJtagReg(addr, value);

	return true;

}				// writeMem32 ()

//-----------------------------------------------------------------------------
//! Read a byte from the OpenRISC 1000 main memory

//! All we can get are 32-bits words, so we have to unpick the value.

//! The underlying 32-bit routines take target endian arguments and return
//! target endian results. We need to convert to host endianness to access the
//! relevant byte.

//! @todo Provide code to check if the read was from a valid address.

//! @note Read bytes from memory mapped devices at your peril!

//! @param[in] addr  The address to read from
//! @return  The byte read
//-----------------------------------------------------------------------------
uint8_t DebugUnitSC::readMem8(uint32_t addr)
{
	uint32_t word = Utils::ttohl(readMem32(addr & 0xfffffffc));
	uint8_t *bytes = (uint8_t *) (&word);
	int offset = addr & 0x3;

	return bytes[offset];

}				// readMem8 ()

//-----------------------------------------------------------------------------
//! Write a byte to the OpenRISC 1000 main memory

//! All we can get are 32-bits words, so we have to read the current value and
//! construct the new value to write back.

//! The underlying 32-bit routines take target endian arguments and return
//! target endian results. We need to convert to host endianness to alter the
//! relevant byte.

//! @note Write bytes to memory mapped devices at your peril!

//! @todo Provide code to check if the write was to a valid address.

//! @param[in] addr   The address to write to
//! @param[in] value  The byte to write

//! @return  True if the write was successful. For now all writes are
//           successful.
//-----------------------------------------------------------------------------
bool DebugUnitSC::writeMem8(uint32_t addr, uint8_t value)
{
	uint32_t currWord = Utils::ttohl(readMem32(addr & 0xfffffffc));
	uint8_t *currBytes = (uint8_t *) (&currWord);
	int offset = addr & 0x3;

	currBytes[offset] = value;

	return writeMem32(addr & 0xfffffffc, Utils::htotl(currWord));

}				// writeMem8 ()

//-----------------------------------------------------------------------------
//! Get the debug interface CPU0 control register value

//! @return  The value in the RISCOP register
//-----------------------------------------------------------------------------
uint32_t DebugUnitSC::readRiscop()
{
	selectDebugModule(OR1K_SC_CPU0);

	int drLen;		// Size of the data register

	uint32_t calc_recv_crc = 0, recv_crc, status_ret;

	drLen = 1 + 4 + 32 + 52 + 4 + 32;

	// Initialize the register fields
	uint64_t *dRegIn = new uint64_t[(drLen + 63) / 64];
	uint64_t *dRegOut = new uint64_t[(drLen + 63) / 64];

	// Write the READ command
	clearBits(dRegIn, drLen);

	packBits(dRegIn, 0, 1, 0);
	packBits(dRegIn, 0 + 1, 4, BITREV(0x3, 4));	// We're reading CPU0 control reg
	uint32_t crc32_send = crc32(dRegIn, 0 + 1 + 4, 0);
	packBits(dRegIn, 0 + 1 + 4, 32, BITREV(crc32_send, 32));

	// Allocate a SystemC completion event
	sc_event *done = new sc_event();

	// Loop until status is OK and CRCs match.
	do {
		TapActionDRScan *dRScan =
		    new TapActionDRScan(done, dRegIn, drLen);
		tapActionQueue->write(dRScan);
		wait(*done);
		dRScan->getDRegOut(dRegOut);
		delete dRScan;
		status_ret = unpackBits(dRegOut, 1 + 4 + 32 + 52, 4);
		calc_recv_crc = crc32(dRegOut, 52 + 4, 1 + 4 + 32);
		recv_crc =
		    BITREV(unpackBits(dRegOut, 1 + 4 + 32 + 52 + 4, 32), 32);
	}
	while ((0 != status_ret) || (calc_recv_crc != recv_crc));

	// All done
	uint32_t res = BITREV(unpackBits(dRegOut, (1 + 4 + 32), 2), 2);

	delete[]dRegIn;
	delete[]dRegOut;
	delete done;
	return res;
}				// readRiscop ()

//-----------------------------------------------------------------------------
//! Set the RISCOP control register

//! Convenience function. Select the REGISTER scan chain, write the new value.

//! @param[in] value  The value to write into the RISCOP register
//-----------------------------------------------------------------------------
void DebugUnitSC::writeRiscop(uint32_t value)
{
	selectDebugModule(OR1K_SC_CPU0);

	int drLen;		// Size of the data register

	uint32_t calc_recv_crc = 0, recv_crc, status_ret;

	drLen = 1 + 4 + 32 + 52 + 4 + 32;

	// Initialize the register fields
	uint64_t *dRegIn = new uint64_t[(drLen + 63) / 64];
	uint64_t *dRegOut = new uint64_t[(drLen + 63) / 64];

	// Write the READ command
	clearBits(dRegIn, drLen);

	packBits(dRegIn, 0, 1, 0);
	packBits(dRegIn, 0 + 1, 4, BITREV(0x4, 4));	// We're writing CPU0 control reg
	packBits(dRegIn, 5, 1, value & RISCOP_RESET);	// First bit is reset
	packBits(dRegIn, 6, 1, (value & RISCOP_STALL) >> 1);	// Next bit is stall
	/* Next 50 bits should be zero */
	uint32_t crc32_send = crc32(dRegIn, 1 + 4 + 52, 0);
	packBits(dRegIn, 1 + 4 + 52, 32, BITREV(crc32_send, 32));

	// Allocate a SystemC completion event
	sc_event *done = new sc_event();

	// Loop until status is OK and CRCs match.
	do {
		TapActionDRScan *dRScan =
		    new TapActionDRScan(done, dRegIn, drLen);
		tapActionQueue->write(dRScan);
		wait(*done);
		dRScan->getDRegOut(dRegOut);
		delete dRScan;
		status_ret = unpackBits(dRegOut, 1 + 4 + 32 + 52, 4);
		calc_recv_crc = crc32(dRegOut, 4, 1 + 4 + 52 + 32);
		recv_crc =
		    BITREV(unpackBits(dRegOut, 1 + 4 + 52 + 32 + 4, 32), 32);
	}
	while ((0 != status_ret) || (calc_recv_crc != recv_crc));

	delete[]dRegIn;
	delete[]dRegOut;
	delete done;

}				// writeRiscop ()

//-----------------------------------------------------------------------------
//! Select a module attached to the debug module

//! @note Must be called from a SystemC thread, because of the use of wait()

//! @param[in] chain  The desired module
//-----------------------------------------------------------------------------
void DebugUnitSC::selectDebugModule(int module)
{

	if (module == currentScanChain) {
		return;
	} else {
		currentScanChain = module;
	}

	sc_event *done = new sc_event();
	TapActionIRScan *iRScan;
	TapActionDRScan *dRScan;

	// Create and queue the IR-Scan action for DEBUG (no CRC)
	iRScan = new TapActionIRScan(done, DEBUG_IR, JTAG_IR_LEN);
	tapActionQueue->write(iRScan);
	wait(*done);

	delete iRScan;

	// Initialize the register fields
	uint64_t *dRegIn = new uint64_t[(DUSEL_DR_LEN + 63) / 64];
	uint64_t *dRegOut = new uint64_t[(DUSEL_DR_LEN + 63) / 64];

	clearBits(dRegIn, DUSEL_DR_LEN);
	packBits(dRegIn, DUSEL_SEL_OFF, DUSEL_SEL_LEN, 0x1);
	packBits(dRegIn, DUSEL_OPCODE_OFF, DUSEL_OPCODE_LEN,
		 bit_reverse_data(module, 4));
	uint32_t crc32_send = crc32(dRegIn, DUSEL_CRC_OFF, 0);
	packBits(dRegIn, DUSEL_CRC_OFF, DUSEL_CRC_LEN,
		 bit_reverse_data(crc32_send, 32));
	uint32_t calc_recv_crc = 0, recv_crc, status_ret;
	// Loop until status is OK and CRCs match.
	do {
		TapActionDRScan *dRScan =
		    new TapActionDRScan(done, dRegIn, DUSEL_DR_LEN);
		tapActionQueue->write(dRScan);
		wait(*done);
		dRScan->getDRegOut(dRegOut);
		delete dRScan;
		status_ret =
		    unpackBits(dRegOut, DUSEL_RESP_STATUS_OFF,
			       DUSEL_RESP_STATUS_LEN);
		calc_recv_crc =
		    crc32(dRegOut, DUSEL_RESP_STATUS_LEN,
			  DUSEL_RESP_STATUS_OFF);
		recv_crc =
		    bit_reverse_data(unpackBits
				     (dRegOut, DUSEL_RESP_CRC_OFF,
				      DUSEL_RESP_CRC_LEN), 32);
	}
	while ((0 != status_ret) || (calc_recv_crc != recv_crc));

	delete[]dRegIn;
	delete[]dRegOut;
	delete done;

}				// selectDebugModule()

//-----------------------------------------------------------------------------
//! Read a 32-bit value via the debug interface

//! @note Must be called from a SystemC thread, because of the use of wait()

//! @param[in] addr  The address of the register

//! @return  The register value read
//-----------------------------------------------------------------------------
uint32_t DebugUnitSC::readJtagReg(uint32_t addr)
{
	int drLen;		// Size of the data register

	uint32_t calc_recv_crc = 0, recv_crc, status_ret;

	drLen = 125;		// Size of write command command (bigger than data read)

	// Initialize the register fields
	uint64_t *dRegIn = new uint64_t[(drLen + 63) / 64];
	uint64_t *dRegOut = new uint64_t[(drLen + 63) / 64];

	// Write the READ command
	clearBits(dRegIn, drLen);

	packBits(dRegIn, 0, 1, 0);
	packBits(dRegIn, 0 + 1, 4, BITREV(0x2, 4));	// We're writing a command
	packBits(dRegIn, 1 + 4, 4, BITREV(0x6, 4));	// Access type, 0x6 = 32-bit READ
	packBits(dRegIn, 1 + 4 + 4, 32, BITREV(addr, 32));	// Address
	packBits(dRegIn, 1 + 4 + 4 + 32, 16, BITREV(0x3, 16));	// Length (always 32-bit,n=(32/8)-1=3)
	uint32_t crc32_send = crc32(dRegIn, 1 + 4 + 4 + 32 + 16, 0);
	packBits(dRegIn, 1 + 4 + 4 + 32 + 16, 32, BITREV(crc32_send, 32));

	// Allocate a SystemC completion event
	sc_event *done = new sc_event();

	// Loop until status is OK and CRCs match.
	do {
		TapActionDRScan *dRScan =
		    new TapActionDRScan(done, dRegIn, 125);
		tapActionQueue->write(dRScan);
		wait(*done);
		dRScan->getDRegOut(dRegOut);
		delete dRScan;
		status_ret = unpackBits(dRegOut, 1 + 4 + 4 + 32 + 16 + 32, 4);
		calc_recv_crc = crc32(dRegOut, 4, 1 + 4 + 4 + 32 + 16 + 32);
		recv_crc =
		    BITREV(unpackBits
			   (dRegOut, 1 + 4 + 4 + 32 + 16 + 32 + 4, 32), 32);
	}
	while ((0 != status_ret) || (calc_recv_crc != recv_crc));

	clearBits(dRegIn, drLen);
	packBits(dRegIn, 0, 1, 0);
	packBits(dRegIn, 0 + 1, 4, 0x0);	// We're GO'ing command
	crc32_send = crc32(dRegIn, 1 + 4, 0);
	packBits(dRegIn, 1 + 4, 32, BITREV(crc32_send, 32));	// CRC

	uint32_t result;
	// Loop until status is OK and CRCs match.
	do {
		TapActionDRScan *dRScan = new TapActionDRScan(done, dRegIn,
							      (1 + 4 + 32 + 36 +
							       ((3 + 1) * 8)));
		tapActionQueue->write(dRScan);
		wait(*done);
		dRScan->getDRegOut(dRegOut);
		delete dRScan;
		status_ret =
		    BITREV(unpackBits(dRegOut, 1 + 4 + 32 + ((3 + 1) * 8), 4),
			   4);
		if (status_ret) {
			printf("readJtagReg(): Addr: 0x%.8x Status err: 0x%x\n",
			       addr, status_ret);
			result = 0;
			break;
		}
		calc_recv_crc = crc32(dRegOut, ((3 + 1) * 8) + 4, 1 + 4 + 32);
		recv_crc =
		    BITREV(unpackBits
			   (dRegOut, 1 + 4 + 32 + ((3 + 1) * 8) + 4, 32), 32);
		result =
		    BITREV(unpackBits(dRegOut, (1 + 4 + 32), ((3 + 1) * 8)),
			   32);

	}
	while ((0 != status_ret) || (calc_recv_crc != recv_crc));

	// All done

	delete[]dRegIn;
	delete[]dRegOut;
	delete done;
	return result;

}				// readJtagReg ()

//-----------------------------------------------------------------------------
//! Write a 32-bit value via the debug interface

//! @note Must be called from a SystemC thread, because of the use of wait()

//! @param[in] addr  The address of the register
//! @param[in] data  The register data to write
//-----------------------------------------------------------------------------
void DebugUnitSC::writeJtagReg(uint32_t addr, uint32_t data)
{
	int drLen;		// Size of the data register

	uint32_t calc_recv_crc = 0, recv_crc, status_ret;

	drLen = 125;		// Size of write command command (bigger than data read)

	// Initialize the register fields
	uint64_t *dRegIn = new uint64_t[(drLen + 63) / 64];
	uint64_t *dRegOut = new uint64_t[(drLen + 63) / 64];

	// Write the READ command
	clearBits(dRegIn, drLen);

	packBits(dRegIn, 0, 1, 0);
	packBits(dRegIn, 0 + 1, 4, BITREV(0x2, 4));	// We're writing a command
	packBits(dRegIn, 1 + 4, 4, BITREV(0x2, 4));	// Access type, 0x2 = 32-bit WRITE
	packBits(dRegIn, 1 + 4 + 4, 32, BITREV(addr, 32));	// Address
	packBits(dRegIn, 1 + 4 + 4 + 32, 16, BITREV(0x3, 16));	// Length (always 32-bit,n=(32/8)-1=3)
	uint32_t crc32_send = crc32(dRegIn, 1 + 4 + 4 + 32 + 16, 0);
	packBits(dRegIn, 1 + 4 + 4 + 32 + 16, 32, BITREV(crc32_send, 32));

	// Allocate a SystemC completion event
	sc_event *done = new sc_event();

	// Loop until status is OK and CRCs match.
	do {
		TapActionDRScan *dRScan =
		    new TapActionDRScan(done, dRegIn, 125);
		tapActionQueue->write(dRScan);
		wait(*done);
		dRScan->getDRegOut(dRegOut);
		delete dRScan;
		status_ret = unpackBits(dRegOut, 1 + 4 + 4 + 32 + 16 + 32, 4);
		calc_recv_crc = crc32(dRegOut, 4, 1 + 4 + 4 + 32 + 16 + 32);
		recv_crc =
		    BITREV(unpackBits
			   (dRegOut, 1 + 4 + 4 + 32 + 16 + 32 + 4, 32), 32);
	}
	while ((0 != status_ret) || (calc_recv_crc != recv_crc));

	clearBits(dRegIn, drLen);
	packBits(dRegIn, 0, 1, 0);
	packBits(dRegIn, 0 + 1, 4, 0x0);	// We're GO'ing command
	packBits(dRegIn, 0 + 1 + 4, 32, BITREV(data, 32));	// Add in data
	crc32_send = crc32(dRegIn, 1 + 4 + 32, 0);
	packBits(dRegIn, 1 + 4 + 32, 32, BITREV(crc32_send, 32));	// CRC

	// Loop until status is OK and CRCs match.
	do {
		TapActionDRScan *dRScan = new TapActionDRScan(done, dRegIn,
							      (1 + 4 +
							       ((3 + 1) * 8) +
							       32 + 36));
		tapActionQueue->write(dRScan);
		wait(*done);
		dRScan->getDRegOut(dRegOut);
		delete dRScan;
		status_ret = unpackBits(dRegOut, 1 + 4 + 32 + 32, 4);
		calc_recv_crc = crc32(dRegOut, 4, 1 + 4 + 32 + 32);
		recv_crc =
		    BITREV(unpackBits(dRegOut, 1 + 4 + 32 + 32 + 4, 32), 32);
	}
	while ((0 != status_ret) || (calc_recv_crc != recv_crc));

	delete[]dRegIn;
	delete[]dRegOut;
	delete done;

}				// writeJtagReg ()

//-----------------------------------------------------------------------------
//! Clear the bits in a data register

//! We always clear whole 64-bit words, not just the minimum number of
//! bytes. It saves all sorts of confusion when debugging code.

//! @note It is the caller's responsibility to make sure the date register
//!       array is large enough.

//! @param[in,out] regArray  The data register to clear
//! @param[in]     regBits  Size of the data register (in bits)
//-----------------------------------------------------------------------------
void DebugUnitSC::clearBits(uint64_t regArray[], int regBits)
{
	memset((char *)regArray, 0, ((regBits + 63) / 64) * 8);

}				// clearBits ()

//-----------------------------------------------------------------------------
//! Set a bit field in a data register

//! The field is cleared, the supplied value masked and then ored into the
//! vector.

//! @note It is the caller's responsibility to make sure the date register
//!       array is large enough.

//! @param[in,out] regArray     The data register
//! @param[in]     fieldOffset  Start of the field (in bits)
//! @param[in]     fieldBits    Size of the field (in bits)
//! @param[in]     fieldVal     Value to set in the field
//-----------------------------------------------------------------------------
void DebugUnitSC::packBits(uint64_t regArray[],
			   int fieldOffset, int fieldBits, uint64_t fieldVal)
{
	fieldVal &= (1ULL << fieldBits) - 1ULL;	// Mask the supplied value

	int startWord = fieldOffset / 64;
	int endWord = (fieldOffset + fieldBits - 1) / 64;

	fieldOffset = fieldOffset % 64;	// Now refers to target word

	// Deal with the startWord. Get enough bits for the mask and put them in the
	// right place
	uint64_t startMask = ((1ULL << fieldBits) - 1ULL) << fieldOffset;

	regArray[startWord] &= ~startMask;
	regArray[startWord] |= fieldVal << fieldOffset;

	// If we were all in one word, we can give up now.
	if (startWord == endWord) {
		return;
	}
	// Deal with the endWord. Get enough bits for the mask. No need to shift
	// these up - they're always at the bottom of the word
	int bitsToDo = (fieldOffset + fieldBits) % 64;
	uint64_t endMask = (1ULL << bitsToDo) - 1ULL;

	regArray[endWord] &= ~endMask;
	regArray[endWord] |= fieldVal >> (fieldBits - bitsToDo);

}				// packBits ()

//-----------------------------------------------------------------------------
//! Extract a bit field from a data register

//! The field is cleared, the supplied value masked and then ored into the
//! vector.

//! @note It is the caller's responsibility to make sure the date register
//!       array is large enough.

//! @param[in,out] regArray     The data register
//! @param[in]     fieldOffset  Start of the field (in bits)
//! @param[in]     fieldBits    Size of the field (in bits)

//! @return  The value in the field
//-----------------------------------------------------------------------------
uint64_t
    DebugUnitSC::unpackBits(uint64_t regArray[], int fieldOffset, int fieldBits)
{
	int startWord = fieldOffset / 64;
	int endWord = (fieldOffset + fieldBits - 1) / 64;

	fieldOffset = fieldOffset % 64;	// Now refers to target word

	// Deal with the startWord. Get enough bits for the mask and put them in the
	// right place
	uint64_t startMask = ((1ULL << fieldBits) - 1ULL) << fieldOffset;
	uint64_t res = (regArray[startWord] & startMask) >> fieldOffset;

	// If we were all in one word, we can give up now.
	if (startWord == endWord) {
		res &= (1ULL << fieldBits) - 1ULL;	// Mask off any unwanted bits
		return res;
	}
	// Deal with the endWord. Get enough bits for the mask. No need to shift
	// these up - they're always at the bottom of the word
	int bitsToDo = (fieldOffset + fieldBits) % 64;
	uint64_t endMask = (1ULL << bitsToDo) - 1ULL;

	res = res | ((regArray[endWord] & endMask) << (fieldBits - bitsToDo));
	res &= (1ULL << fieldBits) - 1ULL;	// Mask off any unwanted bits
	return res;

}				// unpackBits ()

//-----------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------
uint8_t DebugUnitSC::crc8(uint64_t dataArray[], int size)
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

/* Crc of current read or written data.  */
uint32_t crc_r, crc_w = 0;

/* Generates new crc, sending in new bit input_bit */
uint32_t DebugUnitSC::crc32(uint64_t dataArray[], int size, int offset)
{
	uint32_t crc = 0xffffffff;
	for (int i = offset; i < size + offset; i++) {
		uint32_t d =
		    ((dataArray[i / 64] >> (i % 64)) & 1) ? 0xfffffff :
		    0x0000000;
		uint32_t crc_32 = ((crc >> 31) & 1) ? 0xfffffff : 0x0000000;
		crc <<= 1;
		crc = crc ^ ((d ^ crc_32) & DBG_CRC32_POLY);
	}

	return crc;
}

uint32_t DebugUnitSC::bit_reverse_swar_2(uint32_t x)
{
	return (((x & 0xaaaaaaaa) >> 1) | ((x & 0x55555555) << 1));
}

uint32_t DebugUnitSC::bit_reverse_swar_4(uint32_t x)
{
	x = (((x & 0xaaaaaaaa) >> 1) | ((x & 0x55555555) << 1));
	x = (((x & 0xcccccccc) >> 2) | ((x & 0x33333333) << 2));
	return x;
}

uint32_t DebugUnitSC::bit_reverse_swar_8(uint32_t x)
{
	x = (((x & 0xaaaaaaaa) >> 1) | ((x & 0x55555555) << 1));
	x = (((x & 0xcccccccc) >> 2) | ((x & 0x33333333) << 2));
	x = (((x & 0xf0f0f0f0) >> 4) | ((x & 0x0f0f0f0f) << 4));
	return x;
}

uint32_t DebugUnitSC::bit_reverse_swar_16(uint32_t x)
{
	x = (((x & 0xaaaaaaaa) >> 1) | ((x & 0x55555555) << 1));
	x = (((x & 0xcccccccc) >> 2) | ((x & 0x33333333) << 2));
	x = (((x & 0xf0f0f0f0) >> 4) | ((x & 0x0f0f0f0f) << 4));
	x = (((x & 0xff00ff00) >> 8) | ((x & 0x00ff00ff) << 8));
	return x;
}

uint32_t DebugUnitSC::bit_reverse_swar_32(uint32_t x)
{
	x = (((x & 0xaaaaaaaa) >> 1) | ((x & 0x55555555) << 1));
	x = (((x & 0xcccccccc) >> 2) | ((x & 0x33333333) << 2));
	x = (((x & 0xf0f0f0f0) >> 4) | ((x & 0x0f0f0f0f) << 4));
	x = (((x & 0xff00ff00) >> 8) | ((x & 0x00ff00ff) << 8));
	x = (((x & 0xffff0000) >> 16) | ((x & 0x0000ffff) << 16));	// We could be on 64-bit arch!
	return x;
}

uint32_t DebugUnitSC::bit_reverse_data(uint32_t data, int length)
{
	if (length == 2)
		return bit_reverse_swar_2(data);
	if (length == 4)
		return bit_reverse_swar_4(data);
	if (length == 8)
		return bit_reverse_swar_8(data);
	if (length == 16)
		return bit_reverse_swar_16(data);
	if (length == 32)
		return bit_reverse_swar_32(data);
	// Long and laborious way - hopefully never gets called anymore!
	uint32_t reverse = 0;
	for (int i = 0; i < length; i++)
		reverse |= (((data >> i) & 1) << (length - 1 - i));
	return reverse;
}
