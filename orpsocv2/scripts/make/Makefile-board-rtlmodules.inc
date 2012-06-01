
# First we get a list of modules in the RTL path of the board's path.
# Next we check which modules not in the board's RTL path are in the root RTL
# path (modules which can be commonly instantiated, but over which board 
# build-specific versions take precedence.)

# Also generate list of verilog source files

# Paths under board/***/rtl/verilog we wish to exclude when getting modules
BOARD_VERILOG_MODULES_EXCLUDE += include
BOARD_VERILOG_MODULES_EXCLUDE += $(GATELEVEL_MODULES)
BOARD_VERILOG_MODULES_DIR_LIST=$(shell ls $(BOARD_RTL_VERILOG_DIR))
BOARD_EXT_MODULES_DIR_LIST=$(shell ls $(BOARD_EXT_MODULES_DIR))
# Apply exclude to list of modules
BOARD_RTL_VERILOG_MODULES=$(filter-out $(BOARD_VERILOG_MODULES_EXCLUDE),$(BOARD_VERILOG_MODULES_DIR_LIST))
BOARD_EXT_MODULES=$(filter-out $(BOARD_VERILOG_MODULES_EXCLUDE),$(BOARD_EXT_MODULES_DIR_LIST))

# Now get list of modules that we don't have a version of in the board path
COMMON_VERILOG_MODULES_EXCLUDE += include
COMMON_VERILOG_MODULES_EXCLUDE += $(BOARD_RTL_VERILOG_MODULES)
COMMON_VERILOG_MODULES_EXCLUDE += $(BOARD_EXT_MODULES)
COMMON_VERILOG_MODULES_EXCLUDE += $(GATELEVEL_MODULES)

COMMON_RTL_VERILOG_MODULES_DIR_LIST=$(shell ls $(COMMON_RTL_VERILOG_DIR))
COMMON_RTL_VERILOG_MODULES=$(filter-out $(COMMON_VERILOG_MODULES_EXCLUDE), $(COMMON_RTL_VERILOG_MODULES_DIR_LIST))

# List of verilog source files (only .v files!)
# Board RTL modules first
RTL_VERILOG_SRC=$(shell for module in $(BOARD_RTL_VERILOG_MODULES); do if [ -d $(BOARD_RTL_VERILOG_DIR)/$$module ]; then ls $(BOARD_RTL_VERILOG_DIR)/$$module/*.v; fi; done)

# External modules
RTL_VERILOG_SRC +=$(shell for module in $(BOARD_EXT_MODULES); do if [ -d $(BOARD_EXT_MODULES_DIR)/$$module/rtl/verilog ]; then ls $(BOARD_EXT_MODULES_DIR)/$$module/rtl/verilog/*.v; fi; done)

# Common RTL module source
RTL_VERILOG_SRC +=$(shell for module in $(COMMON_RTL_VERILOG_MODULES); do if [ -d $(COMMON_RTL_VERILOG_DIR)/$$module ]; then ls $(COMMON_RTL_VERILOG_DIR)/$$module/*.v; fi; done)

# List of verilog includes from board RTL path - only for rule sensitivity
RTL_VERILOG_INCLUDES=$(shell ls $(BOARD_RTL_VERILOG_INCLUDE_DIR)/*.*)

# Debugging rules

print-board-modules:
	@echo echo; echo "\t### Board verilog modules ###"; echo
	@echo $(BOARD_RTL_VERILOG_MODULES)
	@echo echo "\t### External verilog modules ###"; echo
	@echo $(BOARD_EXT_MODULES)

print-common-modules-exclude:
	@echo echo; echo "\t### Common verilog modules being excluded due to board versions ###"; echo
	@echo "$(COMMON_VERILOG_MODULES_EXCLUDE)"

print-common-modules:
	@echo echo; echo "\t###  Verilog modules from common RTL dir ###"; echo
	@echo $(COMMON_RTL_VERILOG_MODULES)

print-verilog-src:
	@echo echo; echo "\t### Verilog source ###"; echo
	@echo $(RTL_VERILOG_SRC)


ifeq ($(HAVE_VHDL), 1)
# We have some VHDL we should include.

# Currently only supported for board builds - no common VHDL included at present
BOARD_RTL_VHDL_DIR=$(BOARD_RTL_DIR)/vhdl
BOARD_RTL_VHDL_MODULES=$(shell ls $(BOARD_RTL_VHDL_DIR))

#
# VHDL DUT source variables
#
VHDL_FILE_EXT=vhd

RTL_VHDL_SRC=$(shell for module in $(BOARD_RTL_VHDL_MODULES); do if [ -d $(BOARD_RTL_VHDL_DIR)/$$module ]; then ls $(BOARD_RTL_VHDL_DIR)/$$module/*.$(VHDL_FILE_EXT); fi; done)


# Rule for debugging this script
print-vhdl-modules:
	@echo echo; echo "\t### Board VHDL modules ###"; echo
	@echo $(BOARD_RTL_VHDL_MODULES)

print-vhdl-src:
	@echo echo; echo "\t### VHDL modules and source ###"; echo
	@echo "modules: "; echo $(BOARD_RTL_VHDL_MODULES); echo
	@echo "file extension: "$(VHDL_FILE_EXT)
	@echo "source: "$(RTL_VHDL_SRC)


endif