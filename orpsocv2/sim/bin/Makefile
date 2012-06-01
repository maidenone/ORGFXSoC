######################################################################
####                                                              ####
####  ORPSoCv2 Testbenches Makefile                               ####
####                                                              ####
####  Description                                                 ####
####  ORPSoCv2 Testbenches Makefile, containing rules for         ####
####  configuring and running different tests on the current      ####
####  ORPSoC(v2) design.                                          ####
####                                                              ####
####  To do:                                                      ####
####                                                              ####
####  Author(s):                                                  ####
####      - Julius Baxter, julius@opencores.org                   ####
####                                                              ####
####                                                              ####
######################################################################
####                                                              ####
#### Copyright (C) 2009,2010 Authors and OPENCORES.ORG            ####
####                                                              ####
#### This source file may be used and distributed without         ####
#### restriction provided that this copyright statement is not    ####
#### removed from the file and that any derivative work contains  ####
#### the original copyright notice and the associated disclaimer. ####
####                                                              ####
#### This source file is free software; you can redistribute it   ####
#### and/or modify it under the terms of the GNU Lesser General   ####
#### Public License as published by the Free Software Foundation; ####
#### either version 2.1 of the License, or (at your option) any   ####
#### later version.                                               ####
####                                                              ####
#### This source is distributed in the hope that it will be       ####
#### useful, but WITHOUT ANY WARRANTY; without even the implied   ####
#### warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR      ####
#### PURPOSE.  See the GNU Lesser General Public License for more ####
#### details.                                                     ####
####                                                              ####
#### You should have received a copy of the GNU Lesser General    ####
#### Public License along with this source; if not, download it   ####
#### from http://www.opencores.org/lgpl.shtml                     ####
####                                                              ####
######################################################################

# Name of the directory we're currently in
CUR_DIR=$(shell pwd)

# The root path of the whole project
PROJECT_ROOT ?=$(CUR_DIR)/../..

DESIGN_NAME=orpsoc
RTL_TESTBENCH_TOP=$(DESIGN_NAME)_testbench
# Top level files for DUT and testbench
DUT_TOP=$(RTL_VERILOG_DIR)/$(DESIGN_NAME)_top/$(DESIGN_NAME)_top.v
BENCH_TOP=$(BENCH_VERILOG_DIR)/$(DESIGN_NAME)_testbench.v

# Need this for individual test variables to not break
TEST ?= or1200-simple

TESTS ?= or1200-simple \
	or1200-basic \
	or1200-cbasic \
	or1200-dctest \
	or1200-float \
	or1200-mmu  \
	or1200-except \
	or1200-mac \
	or1200-ext \
	or1200-cy	\
	or1200-ov	\
	or1200-sf	\
	or1200-ffl1 \
	or1200-linkregtest \
	or1200-tick \
	or1200-ticksyscall \
	uart-simple

# Gets turned into verilog `define
SIM_TYPE=RTL

# Paths to other important parts of this test suite
RTL_DIR = $(PROJECT_ROOT)/rtl
RTL_VERILOG_DIR = $(RTL_DIR)/verilog
RTL_VERILOG_INCLUDE_DIR = $(RTL_VERILOG_DIR)/include
#RTL_VHDL_DIR = $(RTL_DIR)/vhdl

PROJECT_VERILOG_DEFINES=$(RTL_VERILOG_INCLUDE_DIR)/$(DESIGN_NAME)-defines.v
# Detect technology to use for the simulation
DESIGN_DEFINES=$(shell cat $(PROJECT_VERILOG_DEFINES) | \
	sed s://.*::g | sed s:\`:\#:g | sed 's:^[ ]*::' | \
	awk '{print};/^\#define/{printf "_%s=%s\n",$$2,$$2}' | \
	grep -v PERIOD | cpp -P | sed s:^_::g | sed s:=$$::g )

# Rule to look at what defines are being extracted from main file
print-defines:
	@echo echo; echo "\t### Design defines ###"; echo
	@echo "\tParsing "$(PROJECT_VERILOG_DEFINES)" and exporting:"
	@echo $(DESIGN_DEFINES)

# Simulation directories
SIM_DIR ?=$(PROJECT_ROOT)/sim
SIM_VLT_DIR ?=$(SIM_DIR)/vlt
RTL_SIM_DIR=$(SIM_DIR)
RTL_SIM_RUN_DIR=$(RTL_SIM_DIR)/run
RTL_SIM_BIN_DIR=$(RTL_SIM_DIR)/bin
RTL_SIM_RESULTS_DIR=$(RTL_SIM_DIR)/out

# Testbench paths
BENCH_DIR=$(PROJECT_ROOT)/bench
BENCH_VERILOG_DIR=$(BENCH_DIR)/verilog
BENCH_VERILOG_INCLUDE_DIR=$(BENCH_VERILOG_DIR)/include
#BENCH_VHDL_DIR=$(BENCH_DIR)/vhdl
BENCH_SYSC_DIR=$(BENCH_DIR)/sysc
BENCH_SYSC_SRC_DIR=$(BENCH_SYSC_DIR)/src
BENCH_SYSC_INCLUDE_DIR=$(BENCH_SYSC_DIR)/include


# System software dir
SW_DIR=$(PROJECT_ROOT)/sw
# BootROM code, which generates a verilog array select values
BOOTROM_FILE=bootrom.v
BOOTROM_SW_DIR=$(SW_DIR)/bootrom
BOOTROM_SRC=$(shell ls $(BOOTROM_SW_DIR)/* | grep -v $(BOOTROM_FILE))
BOOTROM_VERILOG=$(BOOTROM_SW_DIR)/$(BOOTROM_FILE)
$(BOOTROM_VERILOG): $(BOOTROM_SRC)
	$(Q)echo; echo "\t### Generating bootup ROM ###"; echo
	$(Q)$(MAKE) -C $(BOOTROM_SW_DIR) $(BOOTROM_FILE)

# Suffix of file to check after each test for the string
TEST_OUT_FILE_SUFFIX=-general.log
TEST_OK_STRING=8000000d

# Dynamically generated verilog file defining configuration for various things
TEST_DEFINES_VLG=test-defines.v
# Set V=1 when calling make to enable verbose output
# mainly for debugging purposes.
ifeq ($(V), 1)
Q=
QUIET=
else
Q ?=@
QUIET=-quiet
endif

# Modelsim variables
MGC_VSIM=vsim
MGC_VLOG_COMP=vlog
MGC_VHDL_COMP=vcom
MODELSIM=modelsim

# Icarus variables
ICARUS_COMPILE=iverilog
ICARUS_RUN=vvp
ICARUS_SCRIPT=icarus.scr
ICARUS_SIM_EXE=vlogsim.elf
ICARUS=icarus

#Default simulator is Icarus Verilog
# Set SIMULATOR=modelsim to use Modelsim
# Set SIMULATOR=ncverilog to use Cadence's NC-Verilog - TODO
# Set SIMULATOR=icarus to use Icarus Verilog (Default)

SIMULATOR ?= $(ICARUS)

# VPI debugging interface variables
VPI_SRC_C_DIR=$(BENCH_VERILOG_DIR)/vpi/c
VPI_SRC_VERILOG_DIR=$(BENCH_VERILOG_DIR)/vpi/verilog
VPI_SRCS=$(shell ls $(VPI_SRC_C_DIR)/*.[ch])

# Modelsim VPI compile variables
MODELTECH_VPILIB=msim_jp_vpi.sl
# Icarus VPI compile target
ICARUS_VPILIB=jp_vpi

#
# Modelsim-specific settings
#
VOPT_ARGS=$(QUIET) -suppress 2241
# If VCD dump is desired, tell Modelsim not to optimise
# away everything.
ifeq ($(VCD), 1)
# If certain versions of modelsim don't have the vopt executable, define
# MGC_NO_VOPT=1 when running.
ifeq ($(MGC_NO_VOPT), 1)
MGC_VSIM_ARGS +=-voptargs="+acc=rnp"
MGC_VOPT_CMD=echo
MGC_VSIM_TGT=orpsoc_testbench
else
VOPT_ARGS=+acc=rnpqv
MGC_VOPT_CMD= vopt $(QUIET) $(RTL_TESTBENCH_TOP) $(VOPT_ARGS) -o tb
MGC_VSIM_TGT=tb
endif

else

ifeq ($(MGC_NO_VOPT), 1)
MGC_VSIM_ARGS += -vopt
MGC_VOPT_CMD=echo
MGC_VSIM_TGT=orpsoc_testbench
else
VOPT_ARGS=+acc=rnpqv
MGC_VOPT_CMD= vopt $(QUIET) $(RTL_TESTBENCH_TOP) $(VOPT_ARGS) -o tb
MGC_VSIM_TGT=tb
endif


endif
# VSIM commands
# Suppressed warnings - 3009: Failed to open $readmemh() file
# Suppressed warnings - 3009: Module 'blah' does not have a `timescale 
# 			directive in effect, but previous modules do.
# Suppressed warnings - 8598: Non-positive replication multiplier inside 
#			concat. Replication will be ignored
MGC_VSIM_ARGS +=  -suppress 7 -suppress 3009 -suppress 8598 -c $(QUIET) \
		-do "set StdArithNoWarnings 1; run -all; exit"
# Options required when VPI option used
ifeq ($(VPI), 1)
MGC_VPI_LIB=$(VPI_SRC_C_DIR)/$(MODELTECH_VPILIB)
MGC_VSIM_ARGS += -pli $(VPI_SRC_C_DIR)/$(MODELTECH_VPILIB)

ICARUS_VPI_LIB=$(VPI_SRC_C_DIR)/$(ICARUS_VPILIB)
ICARUS_VPI_ARGS=-M$(VPI_SRC_C_DIR) -m$(ICARUS_VPILIB)
endif
# Rule to make the VPI library for Modelsim
$(MGC_VPI_LIB): $(VPI_SRCS)
	$(MAKE) -C $(VPI_SRC_C_DIR) $(MODELTECH_VPILIB)

# Rule to make VPI library for Icarus Verilog
$(ICARUS_VPI_LIB): $(VPI_SRCS)
	$(MAKE) -C $(VPI_SRC_C_DIR) $(ICARUS_VPILIB)

# Manually add the VPI bench verilog path
BENCH_VERILOG_SRC_SUBDIRS += $(VPI_SRC_VERILOG_DIR)

#
# Verilog DUT source variables
#
# A list of paths under rtl/verilog we wish to exclude for module searching
VERILOG_MODULES_EXCLUDE=  include components
VERILOG_MODULES_EXCLUDE_LIST_E=$(shell for exclude in \
		$(VERILOG_MODULES_EXCLUDE); do echo "-e $$exclude"; done)
RTL_VERILOG_MODULES=$(shell ls $(RTL_VERILOG_DIR) | grep -v \
			$(VERILOG_MODULES_EXCLUDE_LIST_E) )
# Specific files to exclude, currently none.
#VERILOG_EXCLUDE=
#VERILOG_EXCLUDE_LIST_E=$(shell for exclude in $(VERILOG_EXCLUDE); \
		do echo "-e $$exclude"; done)
# List of verilog source files, minus excluded files
#RTL_VERILOG_SRC=$(shell for module in $(RTL_VERILOG_MODULES); do \
	if [ -d $(RTL_VERILOG_DIR)/$$module ]; then \
		ls $(RTL_VERILOG_DIR)/$$module/*.v | grep -v \
			$(VERILOG_EXCLUDE_LIST_E); \
	fi; done)
# List of verilog source files, ignoring excludes
RTL_VERILOG_SRC=$(shell for module in $(RTL_VERILOG_MODULES); do \
	if [ -d $(RTL_VERILOG_DIR)/$$module ]; then \
		ls $(RTL_VERILOG_DIR)/$$module/*.v; \
	fi; done)

# List of verilog includes
RTL_VERILOG_INCLUDES=$(shell ls $(RTL_VERILOG_INCLUDE_DIR)/*.*)

print-verilog-src:
	@echo echo; echo "\t### Verilog source ###"; echo
	@echo $(RTL_VERILOG_SRC)

# Rules to make RTL we might need
# Expects modules, if they need making, to have their top verilog file to
# correspond to their module name, and the directory should have a make file
# and rule which works for this command.
# Add name of module to this list, currently only does verilog ones.
# Rule 'rtl' is called just before generating DUT modelsim compilation script
RTL_TO_CHECK=
rtl:
	$(Q)for module in $(RTL_TO_CHECK); do \
		$(MAKE) -C $(RTL_VERILOG_DIR)/$$module $$module.v; \
	done

#
# VHDL DUT source variables
#
# VHDL modules
#RTL_VHDL_MODULES=$(shell ls $(RTL_VHDL_DIR))
# VHDL sources
#RTL_VHDL_SRC=$(shell for module in $(RTL_VHDL_MODULES); do \
	if [ -d $(RTL_VHDL_DIR)/$$module ]; then \
		ls $(RTL_VHDL_DIR)/$$module/*.vhd; \
	fi; done)
#print-vhdl-src:
#	@echo echo; echo "\t### VHDL modules and source ###"; echo
#	@echo "modules: "; echo $(RTL_VHDL_MODULES); echo
#	@echo "source: "$(RTL_VHDL_SRC)


# Testbench verilog source
BENCH_VERILOG_SRC=$(shell ls $(BENCH_VERILOG_DIR)/*.v | grep -v \
	$(DESIGN_NAME)_testbench )

print-bench-src:
	$(Q)echo "\tBench verilog source"; \
	echo $(BENCH_VERILOG_SRC)

# Testbench source subdirectory detection
BENCH_VERILOG_SRC_SUBDIRS +=$(shell for file in `ls $(BENCH_VERILOG_DIR)`; do \
	if [ -d $(BENCH_VERILOG_DIR)/$$file ]; then \
		echo $(BENCH_VERILOG_DIR)/$$file; \
	fi; done)

# Compile script generation rules:

modelsim_dut.scr: rtl $(RTL_VERILOG_SRC) $(RTL_VERILOG_INCLUDES) \
			$(BOOTROM_VERILOG)
	$(Q)echo "+incdir+"$(RTL_VERILOG_INCLUDE_DIR) > $@;
	$(Q)echo "+incdir+"$(BOOTROM_SW_DIR) >> $@;
	$(Q)echo "+incdir+"$(BENCH_VERILOG_INCLUDE_DIR) >> $@;
	$(Q)echo "+libext+.v" >> $@;
	$(Q)for module in $(RTL_VERILOG_MODULES); do \
		if [ -d $(RTL_VERILOG_DIR)/$$module ]; then \
			echo "-y " $(RTL_VERILOG_DIR)/$$module >> $@; \
		fi; done
	$(Q)echo >> $@

modelsim_bench.scr: $(BENCH_VERILOG_SRC)
	$(Q)echo "+incdir+"$(BENCH_VERILOG_DIR) > $@;
	$(Q)for path in $(BENCH_VERILOG_SRC_SUBDIRS); do \
		echo "+incdir+"$$path >> $@; \
	done
	$(Q)for path in $(BENCH_VERILOG_SRC_SUBDIRS); do \
		echo "-y "$$path >> $@; \
	done
	$(Q)echo "+incdir+"$(RTL_VERILOG_INCLUDE_DIR) >> $@;
	$(Q)echo "+libext+.v" >> $@;
	$(Q)for vsrc in $(BENCH_VERILOG_SRC); do echo $$vsrc >> $@; done
	$(Q)echo >> $@

# Compile DUT into "work" library
work: modelsim_dut.scr #$(RTL_VHDL_SRC)
	$(Q)if [ ! -e $@ ]; then vlib $@; fi
#	$(Q)echo; echo "\t### Compiling VHDL design library ###"; echo
#	$(Q)vcom -93 $(QUIET) $(RTL_VHDL_SRC)
	$(Q)echo; echo "\t### Compiling Verilog design library ###"; echo
	$(Q)vlog $(QUIET) -f $< $(DUT_TOP)

# Single compile rule
.PHONY : $(MODELSIM)
$(MODELSIM): modelsim_bench.scr $(TEST_DEFINES_VLG) $(MGC_VPI_LIB) work
	$(Q)echo; echo "\t### Compiling testbench ###"; echo
	$(Q)vlog $(QUIET) -nologo -incr $(BENCH_TOP) -f $<
	$(Q)$(MGC_VOPT_CMD)
	$(Q)echo; echo "\t### Launching simulation ###"; echo
	$(Q)vsim $(VOPT_ARGS) $(MGC_VSIM_ARGS) $(MGC_VSIM_TGT)

#
# Icarus Verilog simulator build and run rules
#
.PHONY: $(ICARUS_SCRIPT)
$(ICARUS_SCRIPT):  $(RTL_VERILOG_SRC) $(RTL_VERILOG_INCLUDES) \
		$(BOOTROM_VERILOG) $(BENCH_VERILOG_SRC)
	$(Q)echo "# Icarus Verilog simulation script" > $@
	$(Q)echo "# Auto generated. Any alterations will be written over!" >> $@
	$(Q)echo "+incdir+"$(RTL_VERILOG_INCLUDE_DIR) > $@;
	$(Q)echo "+incdir+"$(BENCH_VERILOG_DIR) >> $@;
	$(Q)echo "+incdir+"$(BOOTROM_SW_DIR) >> $@;        
	$(Q)for path in $(BENCH_VERILOG_SRC_SUBDIRS); do \
		echo "+incdir+"$$path >> $@; \
	done
	$(Q)for path in $(BENCH_VERILOG_SRC_SUBDIRS); do \
		echo "-y "$$path >> $@; \
	done
	$(Q)for module in $(RTL_VERILOG_MODULES); do \
		echo "-y " $(RTL_VERILOG_DIR)/$$module >> $@; \
	done
	$(Q)echo "-y" $(BENCH_VERILOG_DIR) >> $@;
	$(Q)echo $(BENCH_TOP) >> $@;
	$(Q) echo >> $@

# Icarus design compilation rule
$(ICARUS_SIM_EXE): $(ICARUS_SCRIPT) $(TEST_DEFINES_VLG) 
	$(Q)echo; echo "\t### Compiling ###"; echo
	$(Q) $(ICARUS_COMPILE) -s$(RTL_TESTBENCH_TOP) -c $< -o $@

# Icarus simulation run rule
$(ICARUS): $(ICARUS_SIM_EXE) $(ICARUS_VPI_LIB)
	$(Q)echo; echo "\t### Launching simulation ###"; echo
	$(Q) $(ICARUS_RUN) $(ICARUS_VPI_ARGS) -l ../out/$(ICARUS_RUN).log $<



.PHONY: rtl-test
rtl-test: clean-sim-test-sw sw-vmem clean-test-defines $(TEST_DEFINES_VLG) \
	$(SIMULATOR)

# Run an RTL test followed by checking of generated results
rtl-test-with-check: rtl-test
	$(Q)$(MAKE) check-test-log; \
	if [ $$? -ne 0 ]; then \
		echo; echo "\t### "$(TEST)" test FAIL ###"; echo; \
	else \
		echo; echo "\t### "$(TEST)" test OK ###"; echo; \
	fi

# Do check, don't print anything out
rtl-test-with-check-no-print: rtl-test check-test-log

# Main RTL test loop
rtl-tests:
	$(Q)for test in $(TESTS); do \
		export TEST=$$test; \
		$(MAKE) rtl-test-with-check-no-print; \
		if [ $$? -ne 0 ]; then break; fi; \
		echo; echo "\t### $$test test OK ###"; echo; \
	done


.PHONY: check-test-log
check-test-log:
	$(Q)echo "#!/bin/bash" > $@
	$(Q)echo "function check-test-log { if [ \`grep -c -i "$(TEST_OK_STRING)" "$(RTL_SIM_RESULTS_DIR)"/"$(TEST)$(TEST_OUT_FILE_SUFFIX)"\` -gt 0 ]; then return 0; else return 1; fi; }" >> $@
	$(Q)echo "check-test-log" >> $@
	$(Q)chmod +x $@
	$(Q) echo; echo "\t### Checking simulation results for "$(TEST)" test ###"; echo;
	$(Q)./$@

# Include the test-defines.v generation rule
include ../bin/definesgen.inc

#
# Software make rules (called recursively)
#

# Path for the current test
TEST_SW_DIR=$(SW_DIR)/tests/$(shell echo $(TEST) | cut -d "-" -f 1)/sim

# This file name corresponds to the VMEM file the RAM models will attempt to 
# load via $readmemh().
SIM_DIR_VMEM_IMAGE ?=sram.vmem

ifeq ($(USER_ELF),)
ELF_FILE=$(TEST_SW_DIR)/$(TEST).elf
else
ELF_FILE=$(USER_ELF)
ELF_FILE_NOTDIR=$(notdir $(USER_ELF))
ELF_FILE_NOTDIR_BASENAME=$(basename $(ELF_FILE_NOTDIR))
endif

# Rules allowing user to specify a pre-existing VMEM file to load into the 
# simulation, instead of compiling a test.
ifeq ($(USER_VMEM),)
VMEM_FILE=$(TEST_SW_DIR)/$(TEST).vmem
.PHONY: $(VMEM_FILE)


# If USER_ELF was specified, then we need a slightly different way of
# generating the VMEM file from it than a usual test
ifeq ($(USER_ELF),)
$(SIM_DIR_VMEM_IMAGE): $(VMEM_FILE)
	$(Q)if [ -L $@ ]; then unlink $@; fi
	$(Q)if [ -e $@ ]; then rm $@; fi
	$(Q)ln -s $< $@
else
$(SIM_DIR_VMEM_IMAGE): $(USER_ELF)
	$(Q)$(MAKE) -C $(SW_DIR)/lib \
	USER_ELF_BIN=`pwd`/$(ELF_FILE_NOTDIR_BASENAME).bin \
	USER_ELF_VMEM=`pwd`/$(ELF_FILE_NOTDIR_BASENAME).vmem \
	gen-user-elf-vmem
	$(Q)cp -v $(ELF_FILE_NOTDIR_BASENAME).vmem $@

TEST=$(ELF_FILE_NOTDIR_BASENAME)
endif

else
VMEM_FILE=$(USER_VMEM)
USER_VMEM_NOTDIR=$(notdir $(USER_VMEM))
TEST=$(basename $(USER_VMEM_NOTDIR))

$(SIM_DIR_VMEM_IMAGE): $(VMEM_FILE)
	cp -v $< $@

endif




.PHONY : sw-vmem sw-elf
sw-vmem: $(SIM_DIR_VMEM_IMAGE)

$(TEST_SW_DIR)/$(TEST).vmem:
	$(Q) echo; echo "\t### Compiling software ###"; echo;
	$(Q)$(MAKE) -C $(TEST_SW_DIR) $(TEST).vmem

# Compile ELF and copy it here
sw-elf: $(ELF_FILE)
	$(Q)cp -v $< .

$(TEST_SW_DIR)/$(TEST).elf:
	$(Q) echo; echo "\t### Compiling software ###"; echo;
	$(Q)$(MAKE) -C $(TEST_SW_DIR) $(TEST).elf


# Rule to force generation of the processed orpsoc-defines.h file
processed-verilog-headers-in-c-for-vlt:
	$(Q)$(MAKE) -C $(SW_DIR)/lib processed-verilog-headers
# Now copy the file into the Verilated model build path
	$(Q)cp $(SW_DIR)/lib/include/orpsoc-defines.h $(SIM_VLT_DIR)

# Create test software disassembly

sw-dis: $(TEST_SW_DIR)/$(TEST).dis
	$(Q)cp -v $< .

$(TEST_SW_DIR)/$(TEST).dis:
	$(Q)$(MAKE) -C $(TEST_SW_DIR) $(TEST).dis


#
# Cleaning rules
#
clean: clean-sim clean-sim-test-sw clean-bootrom clean-vlt clean-out clean-sw

clean-sim:
	$(Q) echo; echo "\t### Cleaning simulation run directory ###"; echo;
	$(Q)rm -rf *.* lib_* work transcript check-test-log
	$(Q) if [ -e $(VPI_SRC_C_DIR) ]; then \
		$(MAKE) -C $(VPI_SRC_C_DIR) clean; \
	fi

clean-bootrom:
	$(MAKE) -C $(BOOTROM_SW_DIR) clean

clean-out:
	$(Q)rm -rf $(RTL_SIM_RESULTS_DIR)/*.*

# Clean away verilator build path and objects in SystemC path
clean-vlt:
	$(Q)rm -rf $(SIM_VLT_DIR)
	$(Q)$(MAKE) -C $(BENCH_SYSC_SRC_DIR) -f \
		$(BENCH_SYSC_SRC_DIR)/Modules.make clean

clean-test-defines:
	$(Q)rm -f $(TEST_DEFINES_VLG)

clean-sim-test-sw:
	$(Q)if [ -L $(SIM_DIR_VMEM_IMAGE) ]; then unlink $(SIM_DIR_VMEM_IMAGE); fi

clean-sw:
	$(Q) echo; echo "\t### Cleaning simulation sw directories ###"; echo;
	$(Q) $(MAKE) -C $(SW_DIR)/lib distclean

clean-rtl:
	$(Q) echo; echo "\t### Cleaning generated verilog RTL ###"; echo;
	for module in $(RTL_TO_CHECK); do \
		$(MAKE) -C $(RTL_VERILOG_DIR)/$$module clean; \
	done

# Removes any checked out RTL
distclean: clean
	$(Q) echo; echo "\t### Cleaning generated verilog RTL ###"; echo;
	$(Q)for module in $(RTL_TO_CHECK); do \
		$(MAKE) -C $(RTL_VERILOG_DIR)/$$module distclean; \
	done

################################################################################
# Verilator model build rules
################################################################################

VLT_EXE=Vorpsoc_top
VLT_SCRIPT=verilator.scr

# Script for Verilator
$(SIM_VLT_DIR)/$(VLT_SCRIPT): $(RTL_VERILOG_SRC) $(RTL_VERILOG_INCLUDES) \
				$(BOOTROM_VERILOG)
	$(Q)echo "\tGenerating Verilator script"
	$(Q)echo "# Verilator sources script" > $@
	$(Q)echo "# Auto generated. Any alterations will be written over!" >> $@
	$(Q)echo "+incdir+"$(RTL_VERILOG_INCLUDE_DIR) > $@;
	$(Q)echo "+incdir+"$(BOOTROM_SW_DIR) >> $@;        
	$(Q)echo "+incdir+"$(SIM_RUN_DIR) >> $@;
	$(Q)for module in $(RTL_VERILOG_MODULES); do \
		echo "-y " $(RTL_VERILOG_DIR)/$$module >> $@; \
	done
	$(Q)echo $(DUT_TOP) >> $@;
	$(Q) echo >> $@


SYSC_LIB_ARCH_DIR=$(shell ls $$SYSTEMC | grep "lib-")


# List of System C models - use this list to link the sources into the Verilator
# build directory
SYSC_MODELS=OrpsocAccess MemoryLoad

ifdef VLT_LINT
VLT_FLAGS +=--lint-only
endif

ifdef VLT_DEBUG
VLT_DEBUG_COMPILE_FLAGS = -g
# Enabling the following generates a TON of debugging
# when running verilator. Not so helpful.
#VLT_FLAGS = --debug --dump-tree
VLT_SYSC_DEBUG_DEFINE = VLT_DEBUG=1
endif

# This will build a verilator model that will generate profiling information
# suitable for gprof
# Run it through gprof after exection with: gprof Vorpsoc_top > gprof.out
# then run this through the Verilator tool with:
# verilator_profcfunc gprof.out > vprof.out
ifdef VLT_EXECUTION_PROFILE_BUILD
VLT_CPPFLAGS +=-g -pg
# Maybe add these to VLT_CPPFLAGS: -ftest-coverage -fprofile-arcs
VLT_FLAGS +=-profile-cfuncs
endif

# If set on the command line we build the cycle accurate model which will 
# generate verilator-specific profiling information. This is useful for 
# checking the efficiency of the model - not really useful for checking code 
# or the function of the model.
ifdef VLT_DO_PERFORMANCE_PROFILE_BUILD
VLT_CPPFLAGS += -fprofile-generate -fbranch-probabilities -fvpt \
		-funroll-loops -fpeel-loops -ftracer
else
VLT_CPPFLAGS +=-fprofile-use -Wcoverage-mismatch
endif

# Set VLT_IN_GDB=1 when making if going to run the cycle accurate model 
# executable in GDB.
ifdef VLT_IN_GDB
VLT_CPPFLAGS +=-g -O0 
else
# The default optimisation flag applied to all of the cycle accurate model files
VLT_CPPFLAGS +=-O3
endif

# VCD Enabled by default when building, enable it at runtime
VLT_FLAGS +=-trace
TRACE_FLAGS=-DVM_TRACE=1 -I${SYSTEMPERL}/src

# Verilator tuning
# Inlining:
VLT_FLAGS +=--inline-mult 1
# Optimisation option for Verilator scripts
VLT_FLAGS +=-O3
# X-assign - at reset, all signals are set to random values, helps find 
# reset bugs
VLT_FLAGS +=-x-assign unique

VLT_TRACEOBJ = verilated_vcd_c


# This is the list of extra models we'll issue make commands for
# Included is the SystemPerl trace model
SYSC_MODELS_BUILD=$(SYSC_MODELS) $(VLT_TRACEOBJ)

# List of sources for rule sensitivity
SYSC_MODEL_SOURCES=$(shell ls $(BENCH_SYSC_SRC_DIR)/*.cpp)
SYSC_MODEL_SOURCES +=$(shell ls $(BENCH_SYSC_INCLUDE_DIR)/*.h)

VLT_MODULES_OBJS=$(shell for mod in $(SYSC_MODELS_BUILD); do \
		echo $(SIM_VLT_DIR)/$$mod.o; \
	done)

VLT_MODEL_LINKS=$(shell for SYSCMODEL in $(SYSC_MODELS); do \
		echo $(SIM_VLT_DIR)/$$SYSCMODEL.cpp; \
	done) 

# Make Verilator build path if it doesn't exist
$(SIM_VLT_DIR):
	mkdir -p $@

# Dummy files the RTL requires: timescale.v
DUMMY_FILES_FOR_VLT=$(SIM_VLT_DIR)/timescale.v
$(DUMMY_FILES_FOR_VLT):
	$(Q)for file in $@; do if [ ! -e $$file ]; then touch $$file; fi; done

build-vlt: rtl $(SIM_VLT_DIR) $(DUMMY_FILES_FOR_VLT) $(VLT_MODEL_LINKS) \
	processed-verilog-headers-in-c-for-vlt  $(SIM_VLT_DIR)/$(VLT_EXE)

# Main Cycle-accurate build rule
prepare-vlt: build-vlt
	@echo;echo "\tCycle-accurate model compiled successfully"
	@echo;echo "\tRun the executable with the -h option for usage instructions:";echo
	$(SIM_VLT_DIR)/$(VLT_EXE) -h
	@echo;echo

$(SIM_VLT_DIR)/$(VLT_EXE): $(SIM_VLT_DIR)/lib$(VLT_EXE).a \
				$(SIM_VLT_DIR)/OrpsocMain.o
# Final linking of the simulation executable. Order of libraries here is 
# important!
	$(Q)echo; echo "\tGenerating simulation executable"; echo
	$(Q)cd $(SIM_VLT_DIR) && \
		g++ $(VLT_DEBUG_COMPILE_FLAGS) \
		$(VLT_CPPFLAGS) -I$(BENCH_SYSC_INCLUDE_DIR) -I$(SIM_VLT_DIR) \
		-I$(VERILATOR_ROOT)/include -I$(SYSTEMC)/include -o $(VLT_EXE) \
		-L. -L$(BENCH_SYSC_SRC_DIR) -L$(SYSTEMC)/$(SYSC_LIB_ARCH_DIR) \
		OrpsocMain.o -l$(VLT_EXE) -lmodules -lsystemc

# Now compile the top level systemC "testbench" module from the systemC source 
# path
$(SIM_VLT_DIR)/OrpsocMain.o: $(BENCH_SYSC_SRC_DIR)/OrpsocMain.cpp
	@echo; echo "\tCompiling top level SystemC testbench"; echo
	cd $(SIM_VLT_DIR) && \
	g++ $(VLT_DEBUG_COMPILE_FLAGS) $(VLT_CPPFLAGS) $(TRACE_FLAGS) \
	-I$(BENCH_SYSC_INCLUDE_DIR) -I$(SIM_VLT_DIR) \
	-I$(VERILATOR_ROOT)/include -I$(SYSTEMC)/include -c \
	$(BENCH_SYSC_SRC_DIR)/OrpsocMain.cpp 

$(SIM_VLT_DIR)/lib$(VLT_EXE).a: $(SIM_VLT_DIR)/$(VLT_EXE)__ALL.a \
		$(VLT_MODULES_OBJS) $(SIM_VLT_DIR)/verilated.o
# Now archive all of the libraries from verilator witht he other modules we 
# might have
	@echo; echo "\tArchiving libraries into lib"$(VLT_EXE)".a"; echo
	$(Q)cd $(SIM_VLT_DIR) && \
	cp $(VLT_EXE)__ALL.a lib$(VLT_EXE).a && \
	ar rcs lib$(VLT_EXE).a verilated.o; \
	for SYSCMODEL in $(SYSC_MODELS_BUILD); do \
		ar rcs lib$(VLT_EXE).a $$SYSCMODEL.o; \
	done

$(SIM_VLT_DIR)/verilated.o: $(SYSC_MODEL_SOURCES)
	@echo; echo "\tCompiling verilated.o"; echo
	$(Q)cd $(SIM_VLT_DIR) && \
	export CXXFLAGS=$(VLT_DEBUG_COMPILE_FLAGS); \
	export USER_CPPFLAGS="$(VLT_CPPFLAGS)"; \
	export USER_LDDFLAGS="$(VLT_CPPFLAGS)"; \
	$(MAKE) -f $(VLT_EXE).mk verilated.o

print-sysmod-objs:
	$(Q)echo $(VLT_MODULES_OBJS):

$(VLT_MODULES_OBJS):
# Compile the module files
	@echo; echo "\tCompiling SystemC models"
	$(Q)cd $(SIM_VLT_DIR) && \
	for SYSCMODEL in $(SYSC_MODELS_BUILD); do \
		echo;echo "\t$$SYSCMODEL"; echo; \
		export CXXFLAGS=$(VLT_DEBUG_COMPILE_FLAGS); \
		export USER_CPPFLAGS="$(VLT_CPPFLAGS) -I$(BENCH_SYSC_INCLUDE_DIR)"; \
		export USER_LDDFLAGS="$(VLT_CPPFLAGS)"; \
		 $(MAKE) -f $(VLT_EXE).mk $$SYSCMODEL.o; \
	done	

$(SIM_VLT_DIR)/$(VLT_EXE)__ALL.a: $(SIM_VLT_DIR)/$(VLT_EXE).mk \
		$(SYSC_MODEL_SOURCES)
	@echo; echo "\tCompiling main design"; echo
	$(Q)cd $(SIM_VLT_DIR) && \
	export USER_CPPFLAGS="$(VLT_CPPFLAGS)"; \
	export USER_LDDFLAGS="$(VLT_CPPFLAGS)"; \
	$(MAKE) -f $(VLT_EXE).mk $(VLT_EXE)__ALL.a

$(SIM_VLT_DIR)/$(VLT_EXE).mk: $(SIM_VLT_DIR)/$(VLT_SCRIPT) \
		$(BENCH_SYSC_SRC_DIR)/libmodules.a
# Now call verilator to generate the .mk files
	$(Q)echo; echo "\tGenerating makefiles with Verilator"; echo
	$(Q)cd $(SIM_VLT_DIR) && \
	verilator -language 1364-2001 --top-module orpsoc_top -Mdir . -sc \
	$(VLT_FLAGS) -I$(BENCH_SYSC_INCLUDE_DIR) -I$(BENCH_SYSC_SRC_DIR) \
	-f $(SIM_VLT_DIR)/$(VLT_SCRIPT)

# SystemC modules library
$(BENCH_SYSC_SRC_DIR)/libmodules.a:
	@echo; echo "\tCompiling SystemC modules"; echo
	$(Q)export VLT_CPPFLAGS="$(VLT_CPPFLAGS)"; \
	$(MAKE) -C $(BENCH_SYSC_SRC_DIR) \
	-f $(BENCH_SYSC_SRC_DIR)/Modules.make $(VLT_SYSC_DEBUG_DEFINE) 

print-vlt-model-link-paths:
	$(Q)echo $(VLT_MODEL_LINKS)

$(VLT_MODEL_LINKS):
# Link all the required system C model files into the verilator work dir
	$(Q)for SYSCMODEL in $(SYSC_MODELS); do \
		if [ ! -e $(SIM_VLT_DIR)/$$SYSCMODEL.cpp ]; then \
		echo \
	"\tLinking SystemC model $$SYSCMODEL  Verilator model build path"; \
			ln -s $(BENCH_SYSC_SRC_DIR)/$$SYSCMODEL.cpp \
				$(SIM_VLT_DIR)/$$SYSCMODEL.cpp; \
			ln -s $(BENCH_SYSC_INCLUDE_DIR)/$$SYSCMODEL.h \
				$(SIM_VLT_DIR)/$$SYSCMODEL.h; \
		fi; \
	done


################################################################################
# Verilator model test rules
################################################################################

vlt-test: build-vlt clean-sim-test-sw sw-vmem
	$(SIM_VLT_DIR)/$(VLT_EXE) $(TEST)

vlt-tests:
	$(Q)for test in $(TESTS); do \
		export TEST=$$test; \
		$(MAKE) vlt-test; \
		if [ $$? -ne 0 ]; then break; fi; \
		echo; echo "\t### $$test test OK ###"; echo; \
	done



###############################################################################
# Verilator profiled model build rules
###############################################################################
# To run this, first run a "make prepare-vlt-profiled"
# This new make target copies athe results of the profiling back to the right
# paths before we create everything again
###############################################################################
.PHONY: prepare-vlt-profiled
prepare-vlt-profiled: $(SIM_VLT_DIR)/OrpsocMain.gcda \
	clean-vlt-after-profile-run \
	rtl $(SIM_VLT_DIR) $(DUMMY_FILES_FOR_VLT) $(VLT_MODEL_LINKS) \
	$(SIM_VLT_DIR)/$(VLT_EXE)

$(SIM_VLT_DIR)/OrpsocMain.gcda: $(SIM_VLT_DIR)/$(VLT_EXE)-for-profiling
	$(MAKE) -C $(SW_DIR)/apps/dhry dhry.elf NUM_RUNS=5000
	$(SIM_VLT_DIR)/$(VLT_EXE) -f $(SW_DIR)/apps/dhry/dhry.elf

# Clean all compiled things
clean-vlt-after-profile-run:
	$(Q)echo "\tCleaning away compiled cycle-accurate files"
	$(Q)rm -f $(SIM_VLT_DIR)/*.[oa] $(SIM_VLT_DIR)/$(VLT_EXE)
	$(Q)rm -f $(BENCH_SYSC_SRC_DIR)/*.[oa]

#.PHONY: $(SIM_VLT_DIR)/$(VLT_EXE)-for-profiling
$(SIM_VLT_DIR)/$(VLT_EXE)-for-profiling:
	$(MAKE) build-vlt VLT_DO_PERFORMANCE_PROFILE_BUILD=1

.PHONY: vlt-restore-profileoutput
vlt-restore-profileoutput:
	@echo;echo "\tRestoring profiling outputs"; echo
	$(Q)mkdir -p ../vlt
	$(Q)cp /tmp/*.gc* $(SIM_VLT_DIR)
	$(Q)cp /tmp/*.gc* $(BENCH_SYSC_SRC_DIR)


lint-vlt: $(SIM_VLT_DIR) rtl $(DUMMY_FILES_FOR_VLT) $(SIM_VLT_DIR)/$(VLT_SCRIPT)
	$(Q)echo; echo "\tLinting design with Verilator"; echo
	$(Q)cd $(SIM_VLT_DIR) && \
	verilator -language 1364-2001 --top-module orpsoc_top --lint-only \
	-Mdir . -sc $(VLT_FLAGS) -I$(BENCH_SYSC_INCLUDE_DIR) \
	-I$(BENCH_SYSC_SRC_DIR) -f $(SIM_VLT_DIR)/$(VLT_SCRIPT)

################################################################################
# Architectural simulator test rules
################################################################################

ARCH_SIM_EXE ?=or32-elf-sim
ARCH_SIM_CFG ?= ../bin/refdesign-or1ksim.cfg
ARCH_SIM_OPTS ?= -q

.PHONY: rtl-test
sim-test: clean-sim-test-sw sw-elf 
	$(Q)echo; echo "\t### Launching simulation ###"; echo
	$(Q)$(ARCH_SIM_EXE) $(ARCH_SIM_OPTS) -f $(ARCH_SIM_CFG) $(TEST).elf > \
	$(RTL_SIM_RESULTS_DIR)/$(TEST)$(TEST_OUT_FILE_SUFFIX)

# Run tests in simulation, check output
sim-test-with-check: sim-test check-test-log

# Main architectural simulations test loop
sim-tests:
	$(Q)for test in $(TESTS); do \
		export TEST=$$test; \
		$(MAKE) sim-test-with-check; \
		if [ $$? -ne 0 ]; then break; fi; \
		echo; echo "\t### $$test test OK ###"; echo; \
	done
