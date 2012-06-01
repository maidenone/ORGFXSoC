# Variables holding the names and paths to top-level files

# Name of testbench top
RTL_TESTBENCH_TOP_NAME=$(DESIGN_NAME)_testbench

# Paths to top-level files
DUT_TOP_FILE=$(BOARD_RTL_VERILOG_DIR)/$(DESIGN_NAME)_top/$(DESIGN_NAME)_top.v
BENCH_TOP_FILE=$(BOARD_BENCH_VERILOG_DIR)/$(DESIGN_NAME)_testbench.v
