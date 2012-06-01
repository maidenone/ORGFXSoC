# Makefile fragment with some variables global to this board board
# Expects BOARD_ROOT to be set

FPGA_VENDOR=xilinx
BOARD_NAME=ml501
BOARD=$(FPGA_VENDOR)/$(BOARD_NAME)
DESIGN_NAME=orpsoc

# Path down to root of project
PROJECT_ROOT=$(BOARD_ROOT)/../../..

SYNTHESIS_TOOL=xst

export BOARD

include $(PROJECT_ROOT)/scripts/make/Makefile-misc.inc
include $(PROJECT_ROOT)/scripts/make/Makefile-board-paths.inc
include $(PROJECT_ROOT)/scripts/make/Makefile-board-tops.inc
include $(PROJECT_ROOT)/scripts/make/Makefile-board-definesparse.inc

# Check that the Xilinx scripts have been sourced
ifndef XILINX
$(error The XILINX environment variable was not set, \
please run: 'source /path/to/xilinx_ise/settings{32|64}.sh')
endif

# Backend directories
# This one is the board build's backend dir.
BOARD_BACKEND_DIR=$(BOARD_ROOT)/backend
BOARD_BACKEND_VERILOG_DIR=$(BOARD_BACKEND_DIR)/rtl/verilog
BOARD_BACKEND_BIN_DIR=$(BOARD_BACKEND_DIR)/bin
# Technology backend (vendor-specific)
TECHNOLOGY_BACKEND_DIR=$(BOARD_ROOT)/../backend
# This path is for the technology library
TECHNOLOGY_LIBRARY_VERILOG_DIR=$(XILINX)/verilog

# Bootrom setup
# BootROM code, which generates a verilog array select values
BOOTROM_FILE=bootrom.v
BOOTROM_SW_DIR=$(BOARD_SW_DIR)/bootrom
BOOTROM_SRC=$(shell ls $(BOOTROM_SW_DIR)/* | grep -v $(BOOTROM_FILE))
BOOTROM_VERILOG=$(BOOTROM_SW_DIR)/$(BOOTROM_FILE)

bootrom: $(BOOTROM_VERILOG)

$(BOOTROM_VERILOG): $(BOOTROM_SRC)
	$(Q)echo; echo "\t### Generating bootup ROM ###"; echo
	$(Q)$(MAKE) -C $(BOOTROM_SW_DIR) $(BOOTROM_FILE)

clean-bootrom:
	$(Q)echo; echo "\t### Cleaning bootup ROM ###"; echo
	$(Q)$(MAKE) -C $(BOOTROM_SW_DIR) clean

include $(PROJECT_ROOT)/scripts/make/Makefile-board-rtlmodules.inc

# "Backend" source file stuff (PLL, RAM macro models.)
BOARD_BACKEND_VERILOG_SRC=$(shell ls $(BOARD_BACKEND_VERILOG_DIR)/*.v )

# Backend tool path

# BACKEND_TECHNOLOGY_VERILOG_SRC should be set if we need to compile specific
# libraries, as in the Actel and Altera case, and left empty for Xilinx who
# allow us to simply pass the path with the -y option because they have each
# bit of the tech library in individual files, and in which case this variable
# should be left unset.

# Keep this variable empty
BACKEND_TECHNOLOGY_VERILOG_SRC=
