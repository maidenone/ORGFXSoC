######################################################################
####                                                              ####
####  ORPSoC Xilinx Synthesis Makefile                            ####
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

# We don't want the usbhostslave module to be pulled in during synthesis because
# we haven't copied the headers to our RTL director
#COMMON_VERILOG_MODULES_EXCLUDE+= usbhostslave


# The root path of the board build
BOARD_ROOT ?=$(CUR_DIR)/../../..
include $(BOARD_ROOT)/Makefile.inc

RTL_TOP ?=$(DESIGN_NAME)_top

SYN_RUN_DIR=$(BOARD_SYN_DIR)/run

TIMESCALE_FILE=timescale.v
SYNDIR_TIMESCALE_FILE=$(SYN_RUN_DIR)/$(TIMESCALE_FILE)
$(SYNDIR_TIMESCALE_FILE):
	$(Q)echo "" > $@

SYN_VERILOG_DEFINES=synthesis-defines.v
SYNDIR_SYN_VERILOG_DEFINES=$(SYN_RUN_DIR)/$(SYN_VERILOG_DEFINES)
$(SYNDIR_SYN_VERILOG_DEFINES):
	$(Q)echo "\`define SYNTHESIS" > $@
	$(Q)echo "\`define XILINX" >> $@
	$(Q)echo "" >> $@

GENERATED_DEFINES = $(BOOTROM_VERILOG)
GENERATED_DEFINES += $(SYNDIR_TIMESCALE_FILE)
GENERATED_DEFINES += $(SYNDIR_SYN_VERILOG_DEFINES)


FPGA_PART ?=xc6slx45-2-csg324
OPT_MODE ?=Speed
OPT_LEVEL ?=2

ifeq ($(V), 1)
XILINX_FLAGS ?=
else
XILINX_FLAGS ?=-intstyle silent
endif

XILINX_XST_FLAGS ?= -power NO -glob_opt AllClockNets -write_timing_constraints NO -cross_clock_analysis NO -slice_utilization_ratio 100 -bram_utilization_ratio 100 -dsp_utilization_ratio 100 -safe_implementation No -fsm_style lut -ram_extract Yes -ram_style Auto -rom_extract Yes -rom_style Auto -auto_bram_packing NO -mux_extract YES -mux_style Auto -decoder_extract YES -priority_extract YES -shreg_extract YES -shift_extract YES -xor_collapse YES -resource_sharing YES -async_to_sync NO -use_dsp48 auto -iobuf YES -max_fanout 100000 -bufg 32 -register_duplication YES -equivalent_register_removal YES -register_balancing No -slice_packing YES -optimize_primitives NO -use_clock_enable Auto -use_sync_set Auto -use_sync_reset Auto -iob Auto -slice_utilization_ratio_maxmargin 5

XCF_FILE=$(DESIGN_NAME).xcf
XST_FILE=$(DESIGN_NAME).xst
PRJ_FILE=$(DESIGN_NAME).prj
NGC_FILE=$(DESIGN_NAME).ngc
NETLIST_FILE=$(DESIGN_NAME).v

COREGEN_DIR=$(BOARD_SYN_DIR)/coregen
COREGEN_CGP_FILE=$(COREGEN_DIR)/coregen.cgp
COREGEN_XCO_FILES=$(shell ls $(COREGEN_DIR)/*.xco)

XST_PRJ_FILE_SRC_DECLARE=verilog work

print-config:
	$(Q)echo; echo "\t### Synthesis make configuration ###"; echo
	$(Q)echo "\tFPGA_PART="$(FPGA_PART)
	$(Q)echo "\tOPT_MODE="$(OPT_MODE)
	$(Q)echo "\tOTP_LEVEL="$(OPT_LEVEL)
	$(Q)echo "\tXILINX_XST_FLAGS="$(XILINX_XST_FLAGS)
	$(Q)echo

all: $(NGC_FILE)

# Generate the .xst file
# See this page for information on options:
# http://www.xilinx.com/itp/xilinx4/data/docs/xst/command_line5.html
$(XST_FILE):
	$(Q)echo; echo "\t#### Generating XST file ####"; echo
	$(Q)echo "# XST Script for ORPSoC Synthesis" > $@
	$(Q)echo "# This file is autogenerated - any changes will be overwritten" >> $@
	$(Q)echo "# See the Makefile in syn/xst/bin to make changes" >> $@
	$(Q)echo "run" >> $@
	$(Q)echo "-ifn "$(PRJ_FILE) >> $@
	$(Q)echo "-ifmt mixed" >> $@
	$(Q)echo "-top "$(RTL_TOP) >> $@
	$(Q)echo "-ofmt NGC" >> $@
	$(Q)echo "-ofn "$(NGC_FILE) >> $@
	$(Q)echo "-p "$(FPGA_PART) >> $@
	$(Q)echo "-opt_level "$(OPT_LEVEL) >> $@
	$(Q)echo "-opt_mode "$(OPT_MODE) >> $@
	$(Q)echo "-uc "$(XCF_FILE) >> $@
#	$(Q)echo "elaborate " >> $@
#	$(Q)echo -n "-vlgpath \"" >> $@
# option missing from XST - wtf?!	$(Q)for vlogpath in $(VERILOG_SRC_PATHS); do \
		echo -n $$vlogpath" "; done >> $@
#	$(Q)echo "\"" >> $@
# Give board then common verilog include paths, hoping xst does a sensible thing
# and searches them in order.
	$(Q)echo "-vlgincdir { "$(BOARD_RTL_VERILOG_INCLUDE_DIR)" "$(COMMON_RTL_VERILOG_DIR)/include" "$(BOOTROM_SW_DIR) " "$(DEVICE_TREE_DIR)" }" >> $@
	$(Q)echo >> $@

# Generate Xilinx project (.prj) file
$(PRJ_FILE): $(RTL_VERILOG_SRC)
	$(Q)echo; echo "\t#### Generating Xilinx PRJ file ####";
#	$(Q)echo "# Autogenerated XST .prj file" > $@
#	$(Q)echo "# Any changes will be written over." >> $@
	$(Q)for file in $(RTL_VERILOG_SRC); do \
		echo $(XST_PRJ_FILE_SRC_DECLARE) $$file >> $@ ; \
	done
	$(Q)echo >> $@
	$(Q)echo

# Constraints file
$(XCF_FILE):
	$(Q)echo; echo "\t#### Generating Xilinx XCF file ####"; echo
	$(Q)echo "# Autogenerated XST .prj file" > $@
	$(Q)echo "#" >> $@
	$(Q)echo "# Not much here, XST is smart enough to determine clocks through DCMs" >> $@
	$(Q)echo "#" >> $@
	$(Q)echo "# 100 MHz user clock" >> $@
	$(Q)echo "NET \"sys_clk_in\" TNM_NET = \"sys_clk_in_grp\";" >> $@
	$(Q)echo "TIMESPEC \"TS_sys_clk_in\" = PERIOD \"sys_clk_in_grp\" 10 ns HIGH 50%;" >> $@
	$(Q)echo "# Ignore the reset logic" >> $@
	$(Q)echo "NET rst_n_pad_i* TIG;" >> $@
	$(Q)echo "# Define the two clock domains as timespecs" >> $@
	$(Q)echo "#NET dcm0_clkdv TNM_NET=\"wb_clk\";" >> $@
	$(Q)echo "#TIMESPEC \"TS_wb_clk\" = PERIOD \"wb_clk\" 20 ns HIGH 10;" >> $@

# Generate coregen cores
coregen:
	$(Q)echo; echo "#### Running CORE Gen ####"; echo
	$(Q)$(shell cp $(COREGEN_XCO_FILES) .)
	$(Q)$(shell cp $(COREGEN_CGP_FILE) .)
	$(Q)for file in $(COREGEN_XCO_FILES); do \
		coregen -b $(notdir $$file) -p $(notdir $(COREGEN_CGP_FILE)) $(XILINX_FLAGS); \
	done

# XST command
$(NGC_FILE): $(PRJ_FILE) $(XST_FILE) $(XCF_FILE) $(GENERATED_DEFINES) coregen
	$(Q)echo; echo "\t#### Running XST ####"; echo;
	$(Q)xst -ifn $(XST_FILE) $(XILINX_FLAGS) $(XST_FLAGS)
	$(Q)echo

netlist: $(NETLIST_FILE)

# Netlist generation command
$(NETLIST_FILE): $(NGC_FILE)
	$(Q)echo; echo "\t#### Generating verilog netlist ####"; echo;
	$(Q)netgen -sim -aka -dir . -ofmt verilog $< -w $@


clean:
	$(Q)rm -rf *.* xst tmp _xmsgs xlnx_auto* *.lso

clean-sw:
	$(MAKE) -C $(PROJECT_ROOT)/sw/lib distclean

distclean: clean-sw clean


.PRECIOUS : $(NGC_FILE) $(XST_FILE) $(XCF_FILE)
