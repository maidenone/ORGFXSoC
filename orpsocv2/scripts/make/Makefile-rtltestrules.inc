# RTL test rules


print-tests:
	@echo; echo; echo "\t### Software tests to be run ###"; echo;
	@echo $(TESTS)
	@echo


# Suffix of file to check after each test for the string
TEST_OUT_FILE_SUFFIX=-general.log
TEST_OK_STRING=8000000d

.PHONY: rtl-test
rtl-test: clean-sim-test-sw sw clean-test-defines $(TEST_DEFINES_VLG) \
	$(SIMULATOR)

# Run an RTL test followed by checking of generated results
rtl-test-with-check: rtl-test
	$(Q)$(MAKE) check-test-log; \
	if [ $$? -ne 0 ]; then \
		echo; echo "\t### "$(TEST)" test FAIL ###"; echo; \
	else \
		echo; echo "\t### "$(TEST)" test OK ###"; echo; \
	fi

# Do check, don't print anything out
rtl-test-with-check-no-print: rtl-test check-test-log

# Main RTL test loop
rtl-tests:
	$(Q)for test in $(TESTS); do \
		export TEST=$$test; \
		$(MAKE) rtl-test-with-check-no-print; \
		if [ $$? -ne 0 ]; then break; fi; \
		echo; echo "\t### $$test test OK ###"; echo; \
	done


.PHONY: check-test-log
check-test-log:
	$(Q)echo "#!/bin/bash" > $@
	$(Q)echo "function check-test-log { if [ \`grep -c -i "$(TEST_OK_STRING)" "$(RTL_SIM_RESULTS_DIR)"/"$(TEST)$(TEST_OUT_FILE_SUFFIX)"\` -gt 0 ]; then return 0; else return 1; fi; }" >> $@
	$(Q)echo "check-test-log" >> $@
	$(Q)chmod +x $@
	$(Q) echo; echo "\t### Checking simulation results for "$(TEST)" test ###"; echo;
	$(Q)./$@
