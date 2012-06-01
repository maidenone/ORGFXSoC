######################################################################
####                                                              ####
####  ORPSoCv2 Xilinx simulation Makefile                         ####
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
#### Copyright (C) 2009,2010,2011 Authors and OPENCORES.ORG       ####
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

BOARD_ROOT=$(CUR_DIR)/../..
include $(BOARD_ROOT)/Makefile.inc

# Simulation-specific paths and files from this one
include $(PROJECT_ROOT)/scripts/make/Makefile-board-benchsrc.inc

TEST ?= or1200-simple
TESTS ?= or1200-simple or1200-cbasic or1200-dctest or1200-float or1200-mmu or1200-basic or1200-except or1200-tick or1200-ticksyscall uart-simple

include $(PROJECT_ROOT)/scripts/make/Makefile-simulators.inc

# Gets turned into verilog `define
SIM_TYPE=RTL

SIMULATOR ?= $(MODELSIM)

# Include the test-defines.v generation rule
include $(PROJECT_ROOT)/scripts/make/Makefile-sim-definesgen.inc

#
# Modelsim make rules for RTL tests
#
include $(PROJECT_ROOT)/scripts/make/Makefile-board-modelsim.inc

#
# RTL test rules
#
include $(PROJECT_ROOT)/scripts/make/Makefile-rtltestrules.inc

#
# Software make rules (called recursively)
#
include $(PROJECT_ROOT)/scripts/make/Makefile-board-sw.inc

#
# Cleaning rules
#
include $(PROJECT_ROOT)/scripts/make/Makefile-board-simclean.inc
