# Main defines file is from board include path
PROJECT_VERILOG_DEFINES ?=$(BOARD_RTL_VERILOG_INCLUDE_DIR)/$(DESIGN_NAME)-defines.v

# Detect technology to use for the simulation
DESIGN_DEFINES ?=$(shell cat $(PROJECT_VERILOG_DEFINES) | sed s://.*::g | sed s:\`:\#:g | sed 's:^[ ]*::' | awk '{print};/^\#define/{printf "_%s=%s\n",$$2,$$2}' | grep -v PERIOD | cpp -P | sed s:^_::g | sed s:=$$::g )

# Rule to look at what defines are being extracted from main file
print-defines:
	@echo echo; echo "\t### Design defines ###"; echo;
	@echo "\tParsing "$(PROJECT_VERILOG_DEFINES)" and exporting:"
	@echo $(DESIGN_DEFINES)
