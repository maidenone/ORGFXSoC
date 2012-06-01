# A make rule that creates the test defines verilog file.

# Test defines.v file, called recursively, .PHONY to force its generation
.PHONY: $(TEST_DEFINES_VLG)
$(TEST_DEFINES_VLG):
	$(Q)echo "\`define "$(SIM_TYPE)"_SIM" > $@
	$(Q)echo "\`define SIMULATOR_"`echo $(SIMULATOR) | tr  "[:lower:]" "[:upper:]"` >> $@
	$(Q)echo "\`define TEST_NAME_STRING \""$(TEST)"\"" >> $@
	$(Q)if [ ! -z $$VCD ]; \
		then echo "\`define VCD" >> $@; \
	fi
	$(Q)if [ ! -z $$VCD_DELAY ]; \
		then echo "\`define VCD_DELAY "$$VCD_DELAY >> $@; \
	fi
	$(Q)if [ ! -z $$VCD_DEPTH ]; \
		then echo "\`define VCD_DEPTH "$$VCD_DEPTH >> $@; \
	fi
	$(Q)if [ ! -z $$VCD_DELAY_INSNS ]; \
		then echo "\`define VCD_DELAY_INSNS "$$VCD_DELAY_INSNS >> $@; \
	fi
	$(Q)if [ ! -z $$END_TIME ]; \
		then echo "\`define END_TIME "$$END_TIME >> $@; \
	fi
	$(Q)if [ ! -z $$END_INSNS ]; \
		then echo "\`define END_INSNS "$$END_INSNS >> $@; \
	fi
	$(Q)if [ ! -z $$PRELOAD_RAM ]; \
		then echo "\`define PRELOAD_RAM "$$END_TIME >> $@; \
	fi
	$(Q)if [ -z $$DISABLE_PROCESSOR_LOGS ]; \
		then echo "\`define PROCESSOR_MONITOR_ENABLE_LOGS" >> $@; \
	fi
	$(Q)if [ ! -z $$VPI ]; \
		then echo "\`define VPI_DEBUG" >> $@; \
	fi
	$(Q)if [ ! -z $$SIM_QUIET ]; \
		then echo "\`define SIM_QUIET" >> $@; \
	fi
	$(Q)for module in $(GATELEVEL_MODULES); do echo "\`define "$$module"_IS_GATELEVEL " >> $@; done



