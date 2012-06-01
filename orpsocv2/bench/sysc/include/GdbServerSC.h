// ----------------------------------------------------------------------------

// SystemC GDB RSP server: definition

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

#ifndef GDB_SERVER_SC__H
#define GDB_SERVER_SC__H

#include <stdint.h>

#include "systemc"

#include "JtagSC_includes.h"

#include "OrpsocAccess.h"
#include "RspConnection.h"
#include "MpHash.h"
#include "RspPacket.h"
#include "DebugUnitSC.h"

//! Module implementing a GDB RSP server.

//! A thread listens for RSP requests, which are converted to requests to read
//! and write registers, memory or control the CPU in the debug unit

class GdbServerSC:public sc_core::sc_module {
public:

	// Constructor and destructor
	GdbServerSC(sc_core::sc_module_name name,
		    uint32_t _flashStart,
		    uint32_t _flashEnd,
		    int rspPort,
		    sc_core::sc_fifo < TapAction * >*tapActionQueue);
	~GdbServerSC();

private:

	//! Definition of GDB target signals.

	//! Data taken from the GDB 6.8 source. Only those we use defined here.
	enum TargetSignal {
		TARGET_SIGNAL_NONE = 0,
		TARGET_SIGNAL_INT = 2,
		TARGET_SIGNAL_ILL = 4,
		TARGET_SIGNAL_TRAP = 5,
		TARGET_SIGNAL_FPE = 8,
		TARGET_SIGNAL_BUS = 10,
		TARGET_SIGNAL_SEGV = 11,
		TARGET_SIGNAL_ALRM = 14,
		TARGET_SIGNAL_USR2 = 31,
		TARGET_SIGNAL_PWR = 32
	};

	// Register numbering. Matches GDB client
	static const int MAX_SPRS = 0x10000;	//!< Max number of OR1K SPRs
	static const int max_gprs = 32;	//!< Max number of OR1K GPRs

	static const int PPC_REGNUM = max_gprs + 0;	//!< Previous PC
	static const int NPC_REGNUM = max_gprs + 1;	//!< Next PC
	static const int SR_REGNUM = max_gprs + 2;	//!< Supervision Register

	static const int NUM_REGS = max_gprs + 3;	//!< Total GDB registers

	//! Maximum size of a GDB RSP packet
	//static const int  RSP_PKT_MAX  = NUM_REGS * 8 + 1;
	static const int RSP_PKT_MAX = 1024 * 16;

	// OpenRISC exception addresses. Only the ones we need to know about
	static const uint32_t EXCEPT_NONE = 0x000;	//!< No exception
	static const uint32_t EXCEPT_RESET = 0x100;	//!< Reset

	// SPR numbers
	static const uint16_t SPR_NPC = 0x0010;	//!< Next program counter
	static const uint16_t SPR_SR = 0x0011;	//!< Supervision register
	static const uint16_t SPR_PPC = 0x0012;	//!< Previous program counter
	static const uint16_t SPR_GPR0 = 0x0400;	//!< GPR 0

	static const uint16_t SPR_DVR0 = 0x3000;	//!< Debug value register 0
	static const uint16_t SPR_DVR1 = 0x3001;	//!< Debug value register 1
	static const uint16_t SPR_DVR2 = 0x3002;	//!< Debug value register 2
	static const uint16_t SPR_DVR3 = 0x3003;	//!< Debug value register 3
	static const uint16_t SPR_DVR4 = 0x3004;	//!< Debug value register 4
	static const uint16_t SPR_DVR5 = 0x3005;	//!< Debug value register 5
	static const uint16_t SPR_DVR6 = 0x3006;	//!< Debug value register 6
	static const uint16_t SPR_DVR7 = 0x3007;	//!< Debug value register 7

	static const uint16_t SPR_DCR0 = 0x3008;	//!< Debug control register 0
	static const uint16_t SPR_DCR1 = 0x3009;	//!< Debug control register 1
	static const uint16_t SPR_DCR2 = 0x300a;	//!< Debug control register 2
	static const uint16_t SPR_DCR3 = 0x300b;	//!< Debug control register 3
	static const uint16_t SPR_DCR4 = 0x300c;	//!< Debug control register 4
	static const uint16_t SPR_DCR5 = 0x300d;	//!< Debug control register 5
	static const uint16_t SPR_DCR6 = 0x300e;	//!< Debug control register 6
	static const uint16_t SPR_DCR7 = 0x300f;	//!< Debug control register 7  

	static const uint16_t SPR_DMR1 = 0x3010;	//!< Debug mode register 1
	static const uint16_t SPR_DMR2 = 0x3011;	//!< Debug mode register 2
	static const uint16_t SPR_DSR = 0x3014;	//!< Debug stop register
	static const uint16_t SPR_DRR = 0x3015;	//!< Debug reason register

	// SPR masks and offsets
	static const uint32_t SPR_DMR1_ST = 0x00400000;	//!< Single-step trace
	static const uint32_t SPR_DMR2_WGB = 0x003ff000;	//!< W/pt generating B/pt
	static const uint32_t SPR_DMR2_WBS = 0xffc00000;	//!< W/pt B/pt status
	static const uint32_t SPR_DSR_TE = 0x00002000;	//!< Trap
	static const uint32_t SPR_DCR_DP_MASK = 0x00000001;	//!< Debug Pair Present
	static const uint32_t SPR_DCR_CC_MASK = 0x0000000e;	//!< Compare Condition
	static const uint32_t SPR_DCR_SC_MASK = 0x00000010;	//!< Signed Comparison
	static const uint32_t SPR_DCR_CT_MASK = 0x000000e0;	//!< Compare To
	static const uint32_t SPR_DMR2_WGB_SHIFT = 12;	//!< W/pt Generate B/pt

	// DRR (Debug Reason Register) Bits
	static const uint32_t SPR_DRR_RSTE = 0x00000001;	//!< Reset
	static const uint32_t SPR_DRR_BUSEE = 0x00000002;	//!< Bus error
	static const uint32_t SPR_DRR_DPFE = 0x00000004;	//!< Data page fault
	static const uint32_t SPR_DRR_IPFE = 0x00000008;	//!< Insn page fault
	static const uint32_t SPR_DRR_TTE = 0x00000010;	//!< Tick timer
	static const uint32_t SPR_DRR_AE = 0x00000020;	//!< Alignment
	static const uint32_t SPR_DRR_IIE = 0x00000040;	//!< Illegal instruction
	static const uint32_t SPR_DRR_IE = 0x00000080;	//!< Interrupt
	static const uint32_t SPR_DRR_DME = 0x00000100;	//!< DTLB miss
	static const uint32_t SPR_DRR_IME = 0x00000200;	//!< ITLB miss
	static const uint32_t SPR_DRR_RE = 0x00000400;	//!< Range fault
	static const uint32_t SPR_DRR_SCE = 0x00000800;	//!< System call
	static const uint32_t SPR_DRR_FPE = 0x00001000;	//!< Floating point
	static const uint32_t SPR_DRR_TE = 0x00002000;	//!< Trap

	//! RSP Signal valu
	uint32_t rsp_sigval;

	//! Trap instruction for OR32
	static const uint32_t OR1K_TRAP_INSTR = 0x21000001;

	//! Thread ID used by Or1ksim
	static const int OR1KSIM_TID = 1;

	// The bounds of flash memory
	uint32_t flashStart;	//<! Start of flash memory
	uint32_t flashEnd;	//<! End of flash memory

	//! Our associated Debug Unit
	DebugUnitSC *debugUnit;

	//! Our associated RSP interface (which we create)
	RspConnection *rsp;

	//! The packet pointer. There is only ever one packet in use at one time, so
	//! there is no need to repeatedly allocate and delete it.
	RspPacket *pkt;

	//! Hash table for matchpoints
	MpHash *mpHash;

	//! Is the target stopped
	bool targetStopped;

	// SystemC thread to listen for and service RSP requests
	void rspServer();

	// Main RSP request handler
	void rspClientRequest();

	// Handle the various RSP requests
	void rspCheckForException();
	void rspReportException();
	void rspContinue();
	void rspContinue(uint32_t except);
	void rspContinue(uint32_t addr, uint32_t except);
	void rspInterrupt();
	void rspReadAllRegs();
	void rspWriteAllRegs();
	void rspReadMem();
	void rspWriteMem();
	void rspReadReg();
	void rspWriteReg();
	void rspQuery();
	void rspCommand();
	void rspSet();
	void rspRestart();
	void rspStep();
	void rspStep(uint32_t except);
	void rspStep(uint32_t addr, uint32_t except);
	void rspVpkt();
	void rspWriteMemBin();
	void rspRemoveMatchpoint();
	void rspInsertMatchpoint();

	// Convenience wrappers for getting particular registers, which are really
	// SPRs.
	uint32_t readNpc();
	void writeNpc(uint32_t addr);

	uint32_t readGpr(int regNum);
	void writeGpr(int regNum, uint32_t value);

	// Check if we got a message from the or1200 monitor module telling us
	// to stall
	bool checkMonitorPipe();

};				// GdbServerSC ()

#endif // GDB_SERVER_SC__H
