
#
# Software compilation rules used mostly in simulation.
#

# Name of the image the RAM model will attempt to load via Verilog $readmemh
# system function.

# Set PRELOAD_RAM=1 to preload the system memory
ifeq ($(PRELOAD_RAM), 1)
SIM_SW_IMAGE ?=sram.vmem
endif

ifeq ($(SIM_SW_IMAGE),)
SIM_SW_IMAGE ?=flash.in
endif

.PHONY : sw
sw: $(SIM_SW_IMAGE)


flash.in: $(SW_TEST_DIR)/$(TEST).flashin
	$(Q)if [ -L $@ ]; then unlink $@; fi
	$(Q)ln -s $< $@

flash16.in: $(SW_TEST_DIR)/$(TEST).flash16
	$(Q)if [ -L $@ ]; then unlink $@; fi
	$(Q)ln -s $< $@

sram.vmem: $(SW_TEST_DIR)/$(TEST).vmem
	$(Q)if [ -L $@ ]; then unlink $@; fi
	$(Q)ln -s $< $@

.PHONY: $(SW_TEST_DIR)/$(TEST).flashin
$(SW_TEST_DIR)/$(TEST).flashin:
	$(Q) echo; echo "\t### Compiling software ###"; echo;
	$(Q)$(MAKE) -C $(SW_TEST_DIR) $(TEST).flashin

.PHONY: $(SW_TEST_DIR)/$(TEST).vmem
$(SW_TEST_DIR)/$(TEST).vmem:
	$(Q) echo; echo "\t### Compiling software ###"; echo;
	$(Q)$(MAKE) -C $(SW_TEST_DIR) $(TEST).vmem

.PHONY: $(SW_TEST_DIR)/$(TEST).flash16
$(SW_TEST_DIR)/$(TEST).flash16:
	$(Q) echo; echo "\t### Compiling software ###"; echo;
	$(Q)$(MAKE) -C $(SW_TEST_DIR) $(TEST).flash16

# Create test software disassembly

sw-dis: $(SW_TEST_DIR)/$(TEST).dis
	$(Q)cp -v $< .

$(SW_TEST_DIR)/$(TEST).dis:
	$(Q)$(MAKE) -C $(SW_TEST_DIR) $(TEST).dis


#
# Clean rules
#

clean-sim-test-sw:
	$(Q)if [ -e $(SIM_SW_IMAGE) ]; then unlink $(SIM_SW_IMAGE); fi
