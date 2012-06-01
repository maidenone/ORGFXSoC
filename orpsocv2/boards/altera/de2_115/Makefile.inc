# Makefile fragment with some variables global to this board
# Expects BOARD_ROOT to be set

FPGA_VENDOR=altera
FPGA_FAMILY ?="Cyclone IV E"
FPGA_PART ?=EP4CE115F29C7
BOARD_NAME=de2_115
BOARD=$(FPGA_VENDOR)/$(BOARD_NAME)
DESIGN_NAME=orpsoc

# Path down to root of project
PROJECT_ROOT=$(BOARD_ROOT)/../../..

SYNTHESIS_TOOL=quartus

export BOARD

# We have some VHDL
HAVE_VHDL=1

include $(PROJECT_ROOT)/scripts/make/Makefile-misc.inc
include $(PROJECT_ROOT)/scripts/make/Makefile-board-paths.inc
include $(PROJECT_ROOT)/scripts/make/Makefile-board-tops.inc
include $(PROJECT_ROOT)/scripts/make/Makefile-board-definesparse.inc

# Check that the ALTERA_PATH variable is set
ifeq ($(ALTERA_PATH),)
$(error ALTERA_PATH environment variable not set. Set it and rerun)
endif

# Technology-specific paths

# Backend directories
# This one is the board build's backend dir.
BOARD_BACKEND_DIR=$(BOARD_ROOT)/backend
BOARD_BACKEND_VERILOG_DIR=$(BOARD_BACKEND_DIR)/rtl/verilog
# Technology backend (vendor-specific)
TECHNOLOGY_BACKEND_DIR=$(BOARD_ROOT)/../backend
# This path is for the technology library
TECHNOLOGY_LIBRARY_VERILOG_DIR=$(ALTERA_PATH)/quartus/eda/sim_lib

# Altera tools demands to see all the files that are defined in
# `line <line> "filename.v" statements, so we need to declare the
# path to the files of the versatile memory controller
VERSATILE_MEM_INC_DIR=$(BOARD_RTL_VERILOG_DIR)/versatile_mem_ctrl/rtl/verilog

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

# Altera-specific stuff
# "Backend" source file stuff (PLL, RAM macro models.)
BOARD_BACKEND_VERILOG_SRC=$(shell ls $(BOARD_BACKEND_VERILOG_DIR)/*.v )

# BACKEND_TECHNOLOGY_VERILOG_SRC should be set if we need to compile specific
# libraries, as in the Actel and Altera case, and left empty for Xilinx who
# allow us to simply pass the path with the -y option because they have each
# bit of the tech library in individual files, and in which case this variable
# should be left unset.

# technology libraries
BACKEND_TECHNOLOGY_VERILOG_SRC +=$(TECHNOLOGY_LIBRARY_VERILOG_DIR)/altera_mf.v
