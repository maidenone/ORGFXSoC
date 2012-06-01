// ----------------------------------------------------------------------------

// SystemC trace header

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

// $Id: TraceSC.h 302 2009-02-13 17:22:07Z jeremy $

// Drives the Verilator VCD trace interface

#ifndef TRACE_SC__H
#define TRACE_SC__H

#include "Vorpsoc_top.h"

#if VM_TRACE
#include <SpTraceVcdC.h>
#endif

//! Class to drive the Verilator VCD trace interface.

//! Substantive code is only implemented if trace is enabled.

class TraceSC:public sc_core::sc_module {
public:

	// Constructor and destructor
	TraceSC(sc_core::sc_module_name name,
		Vorpsoc_top * _traceTarget, int argc, char *argv[]);
	~TraceSC();

	// Method to drive the trace
	void driveTrace();

	// VCD dump controling vars
	int dump_start_delay, dump_stop_set;
	int dumping_now;
	sc_time dump_start, dump_stop;

	/* The port */
	//sc_in<bool>   clk;

private:

	//! The ORPSoC module we are tracing
	Vorpsoc_top * traceTarget;

#if VM_TRACE
	//! The System Perl Trace file
	SpTraceVcdCFile *spTraceFile;
#endif

	// Set the time resolution
	void setSpTimeResolution(sc_time t);

};				// TraceSC ()

#endif // TRACE_SC__H
