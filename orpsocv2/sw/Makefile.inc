######################################################################
####                                                              ####
#### Common software makefile for inclusion by others             ####
####                                                              ####
######################################################################
####                                                              ####
#### Copyright (C) 2010 Authors and OPENCORES.ORG                 ####
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

OR32_TOOL_PREFIX=or32-elf-

OR32_LD=$(OR32_TOOL_PREFIX)ld
OR32_AS=$(OR32_TOOL_PREFIX)as
OR32_CC=$(OR32_TOOL_PREFIX)gcc
OR32_AR=$(OR32_TOOL_PREFIX)ar
OR32_RANLIB=$(OR32_TOOL_PREFIX)ranlib
OR32_OBJDUMP=$(OR32_TOOL_PREFIX)objdump
OR32_OBJCOPY=$(OR32_TOOL_PREFIX)objcopy

# SW_ROOT should be set by whatever is running this

# Special case for CPU drivers
CPU_DRIVER ?=$(SW_ROOT)/drivers/or1200

DESIGN_NAME ?=orpsoc

# If BOARD is set, we'll first scan that path for which drivers they have
# and will override any locally named driver directories here.
ifdef BOARD
BOARD_PATH=$(SW_ROOT)/../boards/$(BOARD)

ifeq ($(V), 1)
$(info BOARD is set to : $(BOARD_PATH))
endif

BOARD_SPECIFIC_DRIVERS=$(shell if [ -e \
	$(BOARD_PATH)/sw/drivers ]; then \
	ls $(BOARD_PATH)/sw/drivers; \
	fi)
BOARD_SW_TESTS=$(shell if [ -e $(BOARD_PATH)/sw/tests ]; then\
	ls $(BOARD_PATH)/sw/tests; \
	fi)

# Set RTL_VERILOG_INCLUDE_DIR
RTL_VERILOG_INCLUDE_DIR=$(BOARD_PATH)/rtl/verilog/include
#We don't actually want this
#DESIGN_NAME ?=$(strip $(shell echo $(BOARD)|  cut -d '/' -f 2))

endif

COMMON_SW_DRIVERS=$(shell ls $(SW_ROOT)/drivers )
COMMON_SW_DRIVERS_WITHOUT_BOARD_DRIVERS=$(filter-out $(BOARD_SPECIFIC_DRIVERS),$(COMMON_SW_DRIVERS))

#
# Create paths pointing to each driver directory
#
SW_DRIVER_PATHS=$(addprefix $(SW_ROOT)/drivers/,$(COMMON_SW_DRIVERS_WITHOUT_BOARD_DRIVERS))
# If any board paths add them here
SW_DRIVER_PATHS += $(addprefix $(BOARD_PATH)/sw/drivers/,$(BOARD_SPECIFIC_DRIVERS))


# Add /include to each path - the expected include directories
SW_DRIVER_INCLUDE_PATHS=$(addsuffix /include,$(SW_DRIVER_PATHS))
# Now add the -I to the front of each so we can pass this to GCC
INCLUDE_FLAGS=$(addprefix -I,$(SW_DRIVER_INCLUDE_PATHS))


print-driver-paths:
	$(Q)echo "\tDrivers in use from common sw path"; echo;
	@echo $(COMMON_SW_DRIVERS); echo; echo
	$(Q)echo "\tDrivers in use from board sw path"; echo;
	@echo $(BOARD_SPECIFIC_DRIVERS); echo; echo;
	$(Q)echo "\tCommon sw drivers without board drivers"; echo;
	@echo $(COMMON_SW_DRIVERS_WITHOUT_BOARD_DRIVERS); echo; echo;
	$(Q)echo "\tAll drivers and their paths"; echo;
	@echo $(SW_DRIVER_PATHS); echo; echo

print-include-paths:
	$(Q)echo "\tSoftware include paths"; echo;
	@echo $(INCLUDE_FLAGS); echo; echo

# If BOARD_PATH isn't set, then we're not compiling for a board, so use the
# generic board.h include path, otherwise, use that board's
ifdef BOARD_PATH
INCLUDE_FLAGS +=-I$(BOARD_PATH)/sw/board/include
else
INCLUDE_FLAGS +=-I$(SW_ROOT)/board/include
endif

# For now, only apps path is in root sw directory
SW_APPS_PATH=$(SW_ROOT)/apps

VECTORS_OBJ ?=$(CPU_DRIVER)/crt0.o
ORPSOC_LIB ?=$(SW_ROOT)/lib/liborpsoc.a
SUPPORT_LIBS ?=$(ORPSOC_LIB)

# All driver compilations will generate an object file of this name to be
# included into the liborpsoc library
DRIVER_OBJ=compiled.o

# Machine flags - uncomment one or create custom combination of flags
# All hardware flags
#MARCH_FLAGS ?=-mhard-mul -mhard-div -mhard-float
# Hardware integer arith, soft float
MARCH_FLAGS ?=-mhard-mul -mhard-div -msoft-float
# FPGA default - only hardware multiply
#MARCH_FLAGS ?=-mhard-mul -msoft-div -msoft-float
# All software div, mul and FPU
#MARCH_FLAGS ?=-msoft-mul -msoft-div -msoft-float

OR32_CFLAGS ?=-g -nostdlib -O2 $(MARCH_FLAGS) \
		$(INCLUDE_FLAGS) \
		-I$(SW_ROOT)/lib/include \

LINK_SCRIPT_OPT ?=-T$(CPU_DRIVER)/link.ld

OR32_LDFLAGS ?=-L$(SW_ROOT)/lib -lorpsoc -lgcc $(LINK_SCRIPT_OPT) -e 256 
OR32_ARFLAGS ?=-r
# RTL_VERILOG_INCLUDE_DIR *MUST* be set!
# Backup one - default, but may be wrong!
RTL_VERILOG_INCLUDE_DIR ?= $(SW_ROOT)/../rtl/verilog/include

DESIGN_VERILOG_DEFINES=$(RTL_VERILOG_INCLUDE_DIR)/$(DESIGN_NAME)-defines.v
DESIGN_PROCESSED_VERILOG_DEFINES=$(SW_ROOT)/lib/include/$(DESIGN_NAME)-defines.h

OR1200_VERILOG_DEFINES=$(RTL_VERILOG_INCLUDE_DIR)/or1200_defines.v
OR1200_PROCESSED_VERILOG_DEFINES=$(SW_ROOT)/lib/include/or1200-defines.h

PROCESSED_DEFINES +=$(DESIGN_PROCESSED_VERILOG_DEFINES) $(OR1200_PROCESSED_VERILOG_DEFINES) 

# Mother makefile can set this if extra things are needed to be built and
# compild with a certain app.
ELF_DEPENDS ?=

# Set V=1 when calling make to enable verbose output
# mainly for debugging purposes.
ifeq ($(V), 1)
Q=
else
Q ?=@
endif

# Our local utilities
UTILS_BIN2HEX=$(SW_ROOT)/utils/bin2hex
$(UTILS_BIN2HEX):
	$(Q)$(MAKE) -C $(SW_ROOT)/utils bin2hex

UTILS_BIN2VMEM=$(SW_ROOT)/utils/bin2vmem
$(UTILS_BIN2VMEM):
	$(Q)$(MAKE) -C $(SW_ROOT)/utils bin2vmem

processed-verilog-headers: $(PROCESSED_DEFINES)

# Rule to generate C header file from Verilog file with `defines in it
$(DESIGN_PROCESSED_VERILOG_DEFINES): $(DESIGN_VERILOG_DEFINES)
	$(Q)echo; echo "\t### Creating software defines header from verilog defines ###";
	$(Q)echo "//This file is autogenerated from "$<" do not change!" > $@
	$(Q)echo "#ifndef _"$(DESIGN_NAME)"_DEFINES_H_" >> $@
	$(Q)echo "#define _"$(DESIGN_NAME)"_DEFINES_H_" >> $@
	$(Q)cat $< | sed s://.*::g | sed 's/'\`'/'#'/g' >> $@
	$(Q)echo "#endif" >> $@
	$(Q)echo; echo >> $@

# This works (doesn't error), but for now we have to remove all of the numbers
# in verilog format, eg. 8'b0010_0000 or 32'h0000_0f00, or 32'd256 etc. as it's
# not so straight forward to convert these
$(OR1200_PROCESSED_VERILOG_DEFINES): $(OR1200_VERILOG_DEFINES)
	$(Q)echo; echo "\t### Creating OR1200 software defines header from verilog defines ###";
	$(Q)echo "//This file is autogenerated from "$<" do not change!" > $@
	$(Q)echo "#ifndef _OR1200_DEFINES_H_" >> $@
	$(Q)echo "#define _OR1200_DEFINES_H_" >> $@
	$(Q)cat $< | sed s://.*::g | grep -v \'[dhb] | sed 's/'\`'/'#'/g' >> $@
	$(Q)echo "#endif" >> $@
	$(Q)echo; echo >> $@

# Default make
%.flashin: %.bin $(UTILS_BIN2HEX)
	$(Q)$(UTILS_BIN2HEX) $< 1  -size_word -pad $(HEX_IMAGE_PADDING) > $@

%.vmem: %.bin $(UTILS_BIN2VMEM)
	$(Q)$(UTILS_BIN2VMEM) $< > $@

%.flash16: %.bin $(UTILS_BIN2VMEM)
	$(Q)$(UTILS_BIN2VMEM) $< -bpw=2 > $@

%.elf: %.c $(VECTORS_OBJ) $(ELF_DEPENDS) $(SUPPORT_LIBS)
	$(Q)$(OR32_CC) $< $(VECTORS_OBJ) $(ELF_DEPENDS) $(OR32_CFLAGS) \
	$(OR32_LDFLAGS) -o $@

%.elf: %.S $(ELF_DEPENDS) $(SUPPORT_LIBS)
	$(Q)$(OR32_CC) $< $(ELF_DEPENDS) $(OR32_CFLAGS) $(OR32_LDFLAGS) -o $@

%.o: %.S processed-verilog-headers
	$(Q)$(OR32_CC) $(OR32_CFLAGS) -c $< -o $@

%.o: %.c processed-verilog-headers
	$(Q)$(OR32_CC) $(OR32_CFLAGS) -c $< -o $@

COMPILE_SRCS_BASENAMES=$(basename $(COMPILE_SRCS))
COMPILE_OBJS=$(COMPILE_SRCS_BASENAMES:%=%.o)

$(DRIVER_OBJ): $(COMPILE_OBJS)
	$(Q)$(OR32_LD) $(OR32_ARFLAGS) $^ -o $@

# Rule to make all necessary driver objects

$(ORPSOC_LIB): 
	$(Q)echo; echo "\t### Building software support library ###"; echo
	$(Q)$(MAKE) -C $(SW_ROOT)/lib liborpsoc.a

$(VECTORS_OBJ): 
	$(Q)$(MAKE) -C $(CPU_DRIVER) crt0.o


# Get a list of the tests
COMMON_SW_TESTS=$(shell ls $(SW_ROOT)/tests)

# A list of the directories in each test directory (not always the same) and
# get a list of what's in them.
SW_TESTS_SUBDIRS=$(shell for test in $(COMMON_SW_TESTS); do ls -d $(SW_ROOT)/tests/$$test/*; done)

# Add board test paths, if any, to be cleaned too
ifdef BOARD_PATH
SW_TESTS_SUBDIRS +=$(shell for test in $(BOARD_SW_TESTS); do ls -d $(BOARD_PATH)/sw/tests/$$test/*; done)
endif

print-sw-tests:
	$(Q)echo; echo "\tSoftware tests"; echo;
	$(Q)echo $(COMMON_SW_TESTS); echo
	$(Q)echo; echo "\tSoftware tests in board path (if any)"; echo;
	$(Q)echo $(BOARD_SW_TESTS); echo

print-sw-tests-subdirs:
	$(Q)echo; echo "\tSoftware tests subdirs"; echo;
	$(Q)echo $(SW_TESTS_SUBDIRS); echo

# This relies on the local clean rule of each makefile
distclean: clean
	$(Q)for testssubdir in $(SW_TESTS_SUBDIRS); do $(MAKE) -C $$testssubdir clean; done
	$(Q)for dir in $(SW_DRIVER_PATHS); do $(MAKE) -C $$dir clean; done
	$(Q)for dir in `ls $(SW_APPS_PATH)`; do $(MAKE) -C $(SW_ROOT)/apps/$$dir clean; done
	$(Q)for dir in `ls $(SW_APPS_PATH)`; do $(MAKE) -C $(SW_ROOT)/apps/$$dir clean; done
	$(Q)$(MAKE) -C $(SW_ROOT)/lib clean
	$(Q)$(MAKE) -C $(SW_ROOT)/bootrom clean
	$(Q)rm -f $(PROCESSED_DEFINES)
