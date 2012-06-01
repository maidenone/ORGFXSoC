# Makefile fragment with some variables global to this board board
# Expects BOARD_ROOT to be set

FPGA_VENDOR=actel
BOARD_NAME=ordb1a3pe1500
BOARD=$(FPGA_VENDOR)/$(BOARD_NAME)
DESIGN_NAME=orpsoc

# Path down to root of project
PROJECT_ROOT=$(BOARD_ROOT)/../../..

SYNTHESIS_TOOL=synplify

export BOARD

include $(PROJECT_ROOT)/scripts/make/Makefile-misc.inc
include $(PROJECT_ROOT)/scripts/make/Makefile-board-paths.inc
include $(PROJECT_ROOT)/scripts/make/Makefile-board-tops.inc
include $(PROJECT_ROOT)/scripts/make/Makefile-board-definesparse.inc

# Technology-specific paths

# Backend directories
# This one is the board build's backend dir.
BOARD_BACKEND_DIR=$(BOARD_ROOT)/backend
BOARD_BACKEND_VERILOG_DIR=$(BOARD_BACKEND_DIR)/rtl/verilog
# Technology backend (vendor-specific)
TECHNOLOGY_BACKEND_DIR=$(BOARD_ROOT)/../backend
# This path is for the technology library
TECHNOLOGY_LIBRARY_VERILOG_DIR=$(TECHNOLOGY_BACKEND_DIR)/rtl/verilog

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

# Actel-specific stuff
# "Backend" source file stuff (PLL, RAM macro models.)
BOARD_BACKEND_VERILOG_SRC=$(shell ls $(BOARD_BACKEND_VERILOG_DIR)/*.v )

# BACKEND_TECHNOLOGY_VERILOG_SRC should be set if we need to compile specific
# libraries, as in the Actel and Altera case, and left empty for Xilinx who
# allow us to simply pass the path with the -y option because they have each
# bit of the tech library in individual files, and in which case this variable
# should be left unset.

# ProASIC technology library
BACKEND_TECHNOLOGY_VERILOG_SRC=$(shell ls $(TECHNOLOGY_LIBRARY_VERILOG_DIR)/*.v )
