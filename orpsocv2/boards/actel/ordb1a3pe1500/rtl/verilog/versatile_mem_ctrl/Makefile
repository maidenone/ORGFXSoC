REPOS_NAME=versatile_mem_ctrl
REPOS_ADR=http://opencores.org/ocsvn/$(REPOS_NAME)/$(REPOS_NAME)/trunk
# Resulting file we need
TOP_FILE=versatile_mem_ctrl.v
# Dir to checkout in the reposti
REPOS_DIR=rtl
REPOS_FILE_TO_MAKE_NAME=sdr_16.v
REPOS_MAKE_DIR=$(REPOS_DIR)/verilog
FILE_TO_MAKE=$(REPOS_MAKE_DIR)/$(REPOS_FILE_TO_MAKE_NAME)
REVISION= # could be -r 35, for example, to use a specific revision's version

SVN_GET_CMD=co # Could also be export, if we you want an unversioned copy

# Set V=1 when calling make to enable verbose output
# mainly for debugging purposes.
ifeq ($(V), 1)
Q=
QUIET=
else
Q ?=@
QUIET=-quiet
endif

ifeq ($(NO_CHECKOUT), 1)
OUR_DEPENDS=
else
OUR_DEPENDS=$(REPOS_DIR) $(FILE_TO_MAKE)
endif

all: $(TOP_FILE)

# Local copy of previous controller
#$(TOP_FILE): versatile_mem_ctrl_ip.v sdr_16_defines.v  versatile_mem_ctrl_defines.v TwoPortRAM_256x36.v
#	vppreproc +incdir+. versatile_mem_ctrl_ip.v > $@

# Memory controller from repos:
# This renames the top module,so it's exactly the same as the module folder name
$(TOP_FILE): $(OUR_DEPENDS)
	$(Q)touch $(FILE_TO_MAKE)
	$(Q)cat $(FILE_TO_MAKE) | sed 's/module\ versatile_mem_ctrl_top/module\ versatile_mem_ctrl/' > $@

# Force this guy to get at least a once over with make each time
.PHONY: $(FILE_TO_MAKE)
$(FILE_TO_MAKE):
	$(Q)$(MAKE) -C $(REPOS_MAKE_DIR) $(REPOS_FILE_TO_MAKE_NAME)

# Checkout the repository
$(REPOS_DIR):
	$(Q)if [ ! -d $@ ]; then \
		echo; \
		echo "\t### Checking out "$(REPOS_NAME)" from OpenCores ###"; \
		echo; \
		svn $(SVN_GET_CMD) $(REVISION) $(REPOS_ADR)/$@; \
	else \
		if [ $(SVN_GET_CMD) = "co" ]; then \
			echo; \
			echo "\t### Updating "$(REPOS_NAME)" from OpenCores repository ###"; \
			echo; \
			cd $(REPOS_DIR) && svn update; \
		fi; \
	fi

# Just remove the top file, causing it to be remade
clean: clean-repos
#	rm -f $(TOP_FILE)

# Clean the checked out repository
clean-repos:
	$(Q)echo;echo "\t### Cleaning "$(REPOS_NAME)" checkout ###"; echo
	$(Q)if [ -d $(REPOS_DIR) ]; then \
		$(MAKE) -C $(REPOS_MAKE_DIR) clean; \
	fi

# Remove the checked out repository
distclean:
#	$(Q)echo;echo "\t### Removing "$(REPOS_NAME)" checkout ###";echo
#	$(Q)rm -rf $(REPOS_DIR)
