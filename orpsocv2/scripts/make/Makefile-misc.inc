# Miscellaneous rules

# Set V=1 when calling make to enable verbose output
# mainly for debugging purposes.
ifeq ($(V), 1)
Q=
QUIET=
else
Q ?=@
QUIET=-quiet
endif
