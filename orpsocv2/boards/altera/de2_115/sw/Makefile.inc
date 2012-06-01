
# Expecting BOARD_SW_ROOT already set to indicate how far below directory we're
# in the board's software root path is.

# Root from the board's sw/ path
PROJ_ROOT=../../../..

# Figure out actual path the common software directory
SW_ROOT=$(BOARD_SW_ROOT)/$(PROJ_ROOT)/sw

# Set the BOARD to be the path within the board/ path of the project that goes
# to this project.
BOARD=actel/ordb1a3pe1500

# Set RTL_VERILOG_INCLUDE_DIR so software 
RTL_VERILOG_INCLUDE_DIR=$(shell pwd)/$(BOARD_SW_ROOT)/../rtl/verilog/include

# Set the processor capability flags
#MARCH_FLAGS =-mhard-mul -mhard-div -msoft-float
MARCH_FLAGS =-mhard-mul -msoft-div -msoft-float
export MARCH_FLAGS

# Finally include the main software include file

include $(SW_ROOT)/Makefile.inc
