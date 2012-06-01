# Makefile for bootROM Verilog
# We will do it by building the main one, and applying our local board's 
# settings.
# To rebuild after board.h is changed, a clean must be done first.

# Set the path to our board's root directory
BOARD_SW_ROOT=..

include $(BOARD_SW_ROOT)/Makefile.inc

all: bootrom.v

# Copy the one build in the root software path to here.
bootrom.v: $(SW_ROOT)/bootrom/bootrom.v
	$(Q)cp -v $< .

# Export BOARD so the Make script in the root software path knows we're to
# use our board.h file, not theirs.
export BOARD

$(SW_ROOT)/bootrom/bootrom.v:
	$(Q)$(MAKE) -C $(SW_ROOT)/bootrom bootrom.v

clean:
	$(Q)rm -f *.o *.bin *.hex *.in *.dis *.v
	$(Q)$(MAKE) -C $(SW_ROOT)/bootrom clean

