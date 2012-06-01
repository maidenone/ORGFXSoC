######################################################################
####                                                              ####
####  ORPSoC Xilinx backend Makefile                              ####
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

# The root path of the board build
BOARD_ROOT ?=$(CUR_DIR)/../../..
include $(BOARD_ROOT)/Makefile.inc

#
# Options for Xilinx PAR tools
#
FPGA_PART=xc3sd1800a-fg676-4
XILINX_FLAGS=-intstyle silent
XILINX_MAP_FLAGS=-logic_opt off
XILINX_AREA_TARGET = speed
TIMING_REPORT_OPTIONS = -u 1000 -e 1000
SPI_FLASH_SIZE_KBYTES ?=8192
SPI_BOOTLOADER_SW_OFFSET_HEX ?=7c0000

print-config:
	$(Q)echo; echo "\t### Backend make configuration ###"; echo
	$(Q)echo "\tFPGA_PART="$(FPGA_PART)
	$(Q)echo "\tXILINX_FLAGS="$(XILINX_FLAGS)
	$(Q)echo "\tXILINX_MAP_FLAGS="$(XILINX_MAP_FLAGS)
	$(Q)echo "\tXILINX_AREA_TARGET="$(XILINX_AREA_TARGET)
	$(Q)echo "\tTIMING_REPORT_OPTIONS="$(TIMING_REPORT_OPTIONS)
	$(Q)echo "\tSPI_FLASH_SIZE_KBYTES="$(SPI_FLASH_SIZE_KBYTES)
	$(Q)echo "\tSPI_BOOTLOADER_SW_OFFSET_HEX="$(SPI_BOOTLOADER_SW_OFFSET_HEX)

NGC_FILE=$(BOARD_SYN_RUN_DIR)/$(DESIGN_NAME).ngc
NGD_FILE=$(DESIGN_NAME).ngd
UCF_FILE=../bin/$(BOARD_NAME).ucf
MAPPED_NCD=$(DESIGN_NAME)_mapped.ncd
PARRED_NCD=$(DESIGN_NAME).ncd
PCF_FILE=$(DESIGN_NAME).pcf
BIT_FILE=$(DESIGN_NAME).bit
BIT_FILE_FOR_SPI=$(DESIGN_NAME)_spiboot.bit
BATCH_FILE=$(DESIGN_NAME).batch
MCS_FILE=$(DESIGN_NAME).mcs

$(NGC_FILE):
	$(Q)$(MAKE) -C $(BOARD_SYN_RUN_DIR) $(DESIGN_NAME).ngc

$(NGD_FILE): $(UCF_FILE) $(NGC_FILE)
	@echo; echo "\t#### Running NGDBuild ####";
	$(Q)ngdbuild -p $(FPGA_PART) -sd $(BOARD_BACKEND_BIN_DIR) \
	-uc $(UCF_FILE)	$(NGC_FILE) $@

#This target uses Xilinx tools to perform Mapping
$(MAPPED_NCD): $(NGD_FILE)
	@echo; echo "\t#### Mapping ####";
	$(Q) export XIL_MAP_NO_DSP_AUTOREG=1 && \
        export XIL_MAP_ALLOW_ANY_DLL_INPUT=1 && \
	map -p $(FPGA_PART) -detail -pr b -cm ${XILINX_AREA_TARGET} \
	-timing -ol high -w $(XILINX_FLAGS) -o $@ -xe n $(NGD_FILE) $(PCF_FILE)

#This target uses Xilinx tools to Place & Route the design
$(PARRED_NCD): $(MAPPED_NCD)
	@echo; echo "\t#### PAR'ing ####";
	$(Q)par -w -ol high $(XILINX_FLAGS) $< $@ $(PCD_FILE)

#This target uses Xilinx tools to generate a bitstream for download
$(BIT_FILE): $(PARRED_NCD)
	@echo; echo "\t#### Generating .bit file ####";
	$(Q)bitgen -w $(XILINX_FLAGS) -g StartUpClk:JtagClk $< $@

$(BIT_FILE_FOR_SPI): $(PARRED_NCD)
	@echo; echo "\t#### Generating .bit file for SPI load ####";
	$(Q)bitgen -w $(XILINX_FLAGS) -g StartUpClk:CClk $< $@

# Generate MCS with bootloader specified by user, if BOOTLOADER_BIN defined.
ifeq ($(BOOTLOADER_BIN),)
$(MCS_FILE): $(BIT_FILE_FOR_SPI)
	@echo; echo "\t#### Generating .mcs file for SPI load ####";
	$(Q)promgen -spi -p mcs -w -o $@ -s $(SPI_FLASH_SIZE_KBYTES) -u 0 $<
else
$(MCS_FILE): $(BIT_FILE_FOR_SPI)
	@echo; echo "\t#### Generating .mcs file with bootloader for SPI load ####";
	$(Q)promgen -spi -p mcs -w -o $@ -s $(SPI_FLASH_SIZE_KBYTES) -u 0 $< \
	-data_file up $(SPI_BOOTLOADER_SW_OFFSET_HEX) $(BOOTLOADER_BIN)
endif

#this target downloads the bitstream to the target fpga
download: $(BIT_FILE) $(BATCH_FILE)
	$(Q)impact -batch $(BATCH_FILE)

#This target uses netgen to make a simulation netlist
netlist: $(PARRED_NCD)
	@echo; echo "\t#### Generating netlist ####";
	$(Q)netgen -ofmt verilog -sim -dir netlist -pcf $(PCF_FILE) $<

#This one uses TRCE to make a timing report
timingreport: $(PARRED_NCD)
	@echo; echo "\t#### Generating timing report ####";
	$(Q)trce $(TIMING_REPORT_OPTIONS) $<


clean:
	$(Q)rm -rf *.* xlnx_auto* _xmsgs

clean-syn:
	$(Q)$(MAKE) -C $(BOARD_SYN_RUN_DIR) distclean

distclean: clean-syn clean

.PRECIOUS : $(PARRED_NCD) $(MAPPED_NCD) $(NGC_FILE) $(NGD_FILE) $(BIT_FILE) $(BIT_FILE_FOR_SPI)
