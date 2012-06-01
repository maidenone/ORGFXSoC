# Icarus script generation, compile and run rules for board simulations

# If VCD dump is desired, tell Modelsim not to optimise
# away everything.
ifeq ($(VCD), 1)
VVP_ARGS="-vcd"
endif


# VPI debugging interface set up
VPI_SRC_C_DIR=$(COMMON_BENCH_VERILOG_DIR)/vpi/c
VPI_SRCS=$(shell ls $(VPI_SRC_C_DIR)/*.[ch])


icarus_script.scr : $(BOARD_BACKEND_VERILOG_SRC) $(RTL_VERILOG_SRC) \
                    $(RTL_VERILOG_INCLUDES) $(BOOTROM_VERILOG) \
                    $(BOARD_BENCH_VERILOG_SRC) $(COMMON_BENCH_VERILOG_SRC) $(TECHNOLOGY_BACKEND_VERILOG_DIR)
#	$(Q)echo "+incdir+"$(TECHNOLOGY_BACKEND_VERILOG_DIR) >> $@;
	$(Q)echo "+incdir+"$(BOARD_BACKEND_VERILOG_DIR) >> $@;
	$(Q)echo "+incdir+"$(BOARD_BENCH_VERILOG_DIR) >> $@;
	$(Q)echo "-y " $(TECHNOLOGY_LIBRARY_VERILOG_DIR) >> $@;
	$(Q)echo "+incdir+"$(BOARD_RTL_VERILOG_INCLUDE_DIR) >> $@;
	$(Q)echo "+incdir+"$(BOOTROM_SW_DIR) >> $@;
	$(Q)echo "+incdir+"$(BOARD_BENCH_VERILOG_INCLUDE_DIR) >> $@;
	$(Q)for module in $(BOARD_RTL_VERILOG_MODULES); do if [ -d $(BOARD_RTL_VERILOG_DIR)/$$module ]; then echo "-y " $(BOARD_RTL_VERILOG_DIR)/$$module >> $@; fi; done
	$(Q)for module in $(COMMON_RTL_VERILOG_MODULES); do if [ -d $(COMMON_RTL_VERILOG_DIR)/$$module ]; then echo "-y " $(COMMON_RTL_VERILOG_DIR)/$$module >> $@; fi; done
	$(Q)echo "-y " $(BOARD_BACKEND_VERILOG_DIR) >> $@;
	$(Q)echo "+incdir+"$(BOARD_RTL_VERILOG_INCLUDE_DIR) >> $@;
	$(Q)echo "+incdir+"$(BOARD_BENCH_VERILOG_INCLUDE_DIR) >> $@;
	$(Q)echo "+incdir+"$(COMMON_BENCH_VERILOG_INCLUDE_DIR) >> $@;
	$(Q)for path in $(BENCH_VERILOG_SUBDIRS); do echo "+incdir+"$$path >> $@; done
	$(Q)for path in $(BENCH_VERILOG_SUBDIRS); do echo "-y "$$path >> $@; done
	$(Q)echo "+incdir+"$(BOARD_RTL_VERILOG_INCLUDE_DIR) >> $@;
	$(Q)echo "+libext+.v" >> $@;
	$(Q)for vsrc in $(BACKEND_TECHNOLOGY_VERILOG_SRC); do echo $$vsrc >> $@; done
	$(Q)for vsrc in $(BOARD_BACKEND_VERILOG_SRC); do echo $$vsrc >> $@; done
	$(Q)for vsrc in $(BOARD_BENCH_VERILOG_SRC); do echo $$vsrc >> $@; done
	$(Q)for vsrc in $(COMMON_BENCH_VERILOG_SRC); do echo $$vsrc >> $@; done
	$(Q)echo "../../rtl/verilog/versatile_library/versatile_library_ordbcycloneiv.v" >> $@;
	$(Q)echo >> $@


#IVERILOG_ARGS="-Wno-implicit -Wno-portbind -Wno-select-range -Wno-timescale -Wno-infloop -Wno-sensitivity-entire-vector -Wno-sensitivity-entire-array"
IVERILOG_ARGS=""

.PHONY : $(ICARUS)

ifeq ($(FPGA_VENDOR), altera)
$(ICARUS): icarus_script.scr
	$(Q)echo; echo "\t### Compiling testbench ###"; echo
	$(Q)echo; echo ">>iverilog $(IVERILOG_ARGS) -o tb -c $< -s orpsoc_testbench $(BENCH_TOP_FILE)"; echo
	$(Q)iverilog -o orpsoc_testbench -c $< -s orpsoc_testbench $(IVERILOG_ARGS) $(BENCH_TOP_FILE) 
	$(Q)echo; echo "\t### Launching simulation ###"; echo
	$(Q)vvp -l ../out/icarus-sim.log orpsoc_testbench $(VVP_ARGS)
endif
