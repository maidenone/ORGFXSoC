// ----------------------------------------------------------------------------

// SystemC trace 

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

// $Id: TraceSC.cpp 302 2009-02-13 17:22:07Z jeremy $

#include "TraceSC.h"
#include <systemc.h>

using namespace std;

#define DEBUG_TRACESC 1

SC_HAS_PROCESS(TraceSC);

//! Constructor for the trace module

//! @param name           Name of this module, passed to the parent
//!                       constructor.
//! @param _trace_target  ORPSoC module to trace

TraceSC::TraceSC(sc_core::sc_module_name name, Vorpsoc_top * _traceTarget, int argc, char *argv[]):
sc_module(name),
traceTarget(_traceTarget)
{
#if VM_TRACE

	// Setup the name of the VCD dump file
	string dumpNameDefault("vlt-dump.vcd");
	string testNameString;
	string vcdDumpFile;

	// Search through the command line parameters for VCD dump options
	dump_start_delay = 0;
	dump_stop_set = 0;
	int time_val;
	int cmdline_name_found = 0;
	if (argc > 1) {
		for (int i = 1; i < argc; i++) {
			if ((strcmp(argv[i], "-vcd") == 0) ||
			    (strcmp(argv[i], "--vcd") == 0)) {
				testNameString = (argv[i + 1]);
				vcdDumpFile = testNameString;
				cmdline_name_found = 1;
			} else if ((strcmp(argv[i], "-vcdstart") == 0) ||
				   (strcmp(argv[i], "--vcdstart") == 0)) {
				time_val = atoi(argv[i + 1]);
				sc_time dump_start_time(time_val, SC_NS);
				dump_start = dump_start_time;
				if (DEBUG_TRACESC)
					cout <<
					    "TraceSC(): Dump start time set at "
					    << dump_start.to_string() << endl;
				dump_start_delay = 1;
				break;
			} else if ((strcmp(argv[i], "-vcdstop") == 0) ||
				   (strcmp(argv[i], "--vcdstop") == 0)) {
				time_val = atoi(argv[i + 1]);
				sc_time dump_stop_time(time_val, SC_NS);
				dump_stop = dump_stop_time;
				if (DEBUG_TRACESC)
					cout <<
					    "TraceSC(): Dump stop time set at "
					    << dump_stop.to_string() << endl;
				dump_stop_set = 1;
				break;
			}
		}
	}

	if (cmdline_name_found == 0)	// otherwise use our default VCD dump file name
		vcdDumpFile = dumpNameDefault;

	Verilated::traceEverOn(true);

	cout << "* Enabling VCD trace";

	if (dump_start_delay)
		cout << ", on at time " << dump_start.to_string();
	if (dump_stop_set)
		cout << ", off at time " << dump_stop.to_string();

	cout << endl;

	printf("* VCD dumpfile: %s\n", vcdDumpFile.c_str());

	// Establish a new trace with its correct time resolution, and trace to
	// great depth.
	spTraceFile = new SpTraceVcdCFile();
	setSpTimeResolution(sc_get_time_resolution());
	traceTarget->trace(spTraceFile, 99);
	spTraceFile->open(vcdDumpFile.c_str());

	if (dump_start_delay == 1)
		dumping_now = 0;	// We'll wait for the time to dump
	else
		dumping_now = 1;	// Begin with dumping turned on

	// Method to drive the dump on each clock edge
	//SC_METHOD (driveTrace);
	//sensitive << clk;

#endif

}				// TraceSC ()

//! Destructor for the trace module.

//! Used to close the tracefile

TraceSC::~TraceSC()
{
#if VM_TRACE
	spTraceFile->close();
#endif

}				// ~TraceSC ()

//! Method to drive the trace. We're called on every clock edge, and also at
//! initialization (to get initial values into the dump).
void
 TraceSC::driveTrace()
{
#if VM_TRACE

	if (DEBUG_TRACESC)
		cout << "TraceSC(): " << endl;
	if (dumping_now == 0) {
		// Check the time, see if we should enable dumping
		if (sc_time_stamp() >= dump_start) {
			// Give a message
			cout << "* VCD tracing turned on at time " <<
			    dump_start.to_string() << endl;
			dumping_now = 1;
		}
	}

	if (dumping_now == 1)
		spTraceFile->dump(sc_time_stamp().to_double());

	// Should we turn off tracing?
	if ((dumping_now == 1) && (dump_stop_set == 1)) {
		if (sc_time_stamp() >= dump_stop) {
			// Give a message
			cout << "* VCD tracing turned off at time " <<
			    dump_stop.to_string() << endl;
			dumping_now = 0;	// Turn off the dump
		}
	}
#endif

}				// driveTrace()

//! Utility method to set the SystemPerl trace time resolution.

//! This should be automatic, but is missed in Verilator 3.700.

//! @param t  The desired time resolution (as a SC time)

void TraceSC::setSpTimeResolution(sc_time t)
{
#if VM_TRACE

	double secs = t.to_seconds();
	int val;		// Integral value of the precision
	const char *units;	// Units as text

	if (secs < 1.0e-15) {
		cerr << "* VCD time resolution " << secs <<
		    " too small: ignored" << endl;
		return;
	} else if (secs < 1.0e-12) {
		val = secs / 1.0e-15;
		units = "f";
	} else if (secs < 1.0e-9) {
		val = secs / 1.0e-12;
		units = "p";
	} else if (secs < 1.0e-6) {
		val = secs / 1.0e-9;
		units = "n";
	} else if (secs < 1.0e-3) {
		val = secs / 1.0e-6;
		units = "u";
	} else if (secs < 1.0) {
		val = secs / 1.0e-3;
		units = "m";
	} else {
		val = secs;
		units = "s";
	}

	// Val must be a power of 10
	switch (val) {
	case 1:
	case 10:
	case 100:
	case 1000:
	case 10000:
	case 100000:
	case 1000000:
	case 10000000:
	case 100000000:
	case 1000000000:

		break;		// OK

	default:
		cerr << "VCD time resolution " << secs <<
		    " not power of 10: ignored" << endl;
		return;
	}

	// Set the time resolution for the trace file
	char str[32];
	sprintf(str, "%d %s", val, units);
	spTraceFile->spTrace()->set_time_resolution(str);

#endif

}				// setSpTimeResolution()
