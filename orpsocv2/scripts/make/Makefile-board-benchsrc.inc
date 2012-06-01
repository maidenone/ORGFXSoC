
#
# Testbench source
#
BOARD_BENCH_VERILOG_SRC=$(shell ls $(BOARD_BENCH_VERILOG_DIR)/*.v | grep -v $(DESIGN_NAME)_testbench )
BOARD_BENCH_VERILOG_SRC_FILES=$(notdir $(BOARD_BENCH_VERILOG_SRC))

# Now only take the source from the common path that we don't already have in
# our board's
COMMON_BENCH_VERILOG_DIR_LS=$(shell ls $(COMMON_BENCH_VERILOG_DIR)/*.v)
COMMON_BENCH_VERILOG_SRC_FILES=$(notdir $(COMMON_BENCH_VERILOG_DIR_LS))
COMMON_BENCH_VERILOG_SRC_FILTERED=$(filter-out $(BOARD_BENCH_VERILOG_SRC_FILES) $(DESIGN_NAME)_testbench.v,$(COMMON_BENCH_VERILOG_SRC_FILES))
COMMON_BENCH_VERILOG_SRC=$(addprefix $(COMMON_BENCH_VERILOG_DIR)/, $(COMMON_BENCH_VERILOG_SRC_FILTERED))

print-board-bench-src:
	$(Q)echo "\tBoard bench verilog source"; \
	echo $(BOARD_BENCH_VERILOG_SRC)

print-common-bench-src:
	$(Q)echo "\Common bench verilog source"; \
	echo $(COMMON_BENCH_VERILOG_SRC)

# Testbench source subdirectory detection (exclude include, we always use 
# board bench include directory!)
BOARD_BENCH_VERILOG_SUBDIRS=$(shell cd $(BOARD_BENCH_VERILOG_DIR) && ls -d */ | grep -v include)
COMMON_BENCH_VERILOG_SUBDIRS=$(shell cd $(COMMON_BENCH_VERILOG_DIR) && ls -d */ | grep -v include)

# Get rid of ones we have a copy of locally
COMMON_BENCH_VERILOG_SUBDIRS_EXCLUDE_BOARDS=$(filter-out $(BOARD_BENCH_VERILOG_SUBDIRS),$(COMMON_BENCH_VERILOG_SUBDIRS))

# Construct list of paths we will want to include
BENCH_VERILOG_SUBDIRS=$(addprefix $(COMMON_BENCH_VERILOG_DIR)/,$(COMMON_BENCH_VERILOG_SUBDIRS_EXCLUDE_BOARDS))
BENCH_VERILOG_SUBDIRS += $(addprefix $(BOARD_BENCH_VERILOG_DIR)/,$(BOARD_BENCH_VERILOG_SUBDIRS))

# Finally, add include path from local bench path
BENCH_VERILOG_SUBDIRS += $(BOARD_BENCH_VERILOG_DIR)/include

ifeq ($(VPI), 1)
# Manually add the VPI bench verilog path
COMMON_BENCH_VERILOG_SUBDIRS += vpi/verilog
endif

print-board-bench-subdirs:
	$(Q)echo "\tBoard bench subdirectories"; \
	echo $(BOARD_BENCH_VERILOG_SUBDIRS)

print-common-bench-subdirs:
	$(Q)echo "\tCommon bench subdirectories"; \
	echo $(COMMON_BENCH_VERILOG_SUBDIRS)

print-bench-subdirs:
	$(Q)echo "\tBench subdirectories"; \
	echo $(BENCH_VERILOG_SUBDIRS)
