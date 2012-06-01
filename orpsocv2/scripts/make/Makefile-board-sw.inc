#
# Software compile rules for boards (mainly used during simulation)
#

# Path for the current test
# First check for a local copy of the test. If it doesn't exist then we
# default to the software tests in the root directory
TEST_MODULE=$(shell echo $(TEST) | cut -d "-" -f 1)
BOARD_SW_TEST_DIR=$(BOARD_SW_DIR)/tests/$(TEST_MODULE)/sim
COMMON_SW_TEST_DIR=$(COMMON_SW_DIR)/tests/$(TEST_MODULE)/sim
# Do this by testing for the file's existence 
SW_TEST_DIR=$(shell if [ -e $(BOARD_SW_TEST_DIR)/$(TEST).[cS] ]; then echo $(BOARD_SW_TEST_DIR); else echo $(COMMON_SW_TEST_DIR); fi)

print-test-sw-dir:
	@echo; echo "\tTest software is in the following path"; echo;
	@echo $(BOARD_SW_DIR); echo;
	@echo $(BOARD_SW_TEST_DIR); echo;
	@echo $(SW_TEST_DIR); echo;

print-sw-tests:
	$(Q) $(MAKE) -C $(COMMON_SW_DIR)/lib print-sw-tests
	$(Q) $(MAKE) -C $(COMMON_SW_DIR)/lib print-sw-tests-subdirs


include $(PROJECT_ROOT)/scripts/make/Makefile-swrules.inc

#
# Board-specific clean rules
#

clean-sw:
	$(Q) echo; echo "\t### Cleaning simulation sw directories ###"; echo;
	$(Q) $(MAKE) -C $(COMMON_SW_DIR)/lib distclean